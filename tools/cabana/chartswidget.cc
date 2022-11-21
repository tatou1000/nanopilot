#include "tools/cabana/chartswidget.h"

#include <QFutureSynchronizer>
#include <QGraphicsLayout>
#include <QRubberBand>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QtConcurrent>

// ChartsWidget

ChartsWidget::ChartsWidget(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(0, 0, 0, 0);

  // toolbar
  QToolBar *toolbar = new QToolBar(tr("Charts"), this);
  title_label = new QLabel();
  title_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  toolbar->addWidget(title_label);
  toolbar->addWidget(range_label = new QLabel());
  reset_zoom_btn = toolbar->addAction("⟲");
  reset_zoom_btn->setToolTip(tr("Reset zoom (drag on chart to zoom X-Axis)"));
  remove_all_btn = toolbar->addAction("✖");
  remove_all_btn->setToolTip(tr("Remove all charts"));
  dock_btn = toolbar->addAction("");
  main_layout->addWidget(toolbar);
  updateToolBar();

  // charts
  QWidget *charts_container = new QWidget(this);
  charts_layout = new QVBoxLayout(charts_container);
  charts_layout->addStretch();

  QScrollArea *charts_scroll = new QScrollArea(this);
  charts_scroll->setWidgetResizable(true);
  charts_scroll->setWidget(charts_container);
  charts_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  main_layout->addWidget(charts_scroll);

  QObject::connect(dbc(), &DBCManager::DBCFileChanged, this, &ChartsWidget::removeAll);
  QObject::connect(can, &CANMessages::eventsMerged, this, &ChartsWidget::eventsMerged);
  QObject::connect(can, &CANMessages::updated, this, &ChartsWidget::updateState);
  QObject::connect(remove_all_btn, &QAction::triggered, this, &ChartsWidget::removeAll);
  QObject::connect(reset_zoom_btn, &QAction::triggered, this, &ChartsWidget::zoomReset);
  QObject::connect(dock_btn, &QAction::triggered, [this]() {
    emit dock(!docking);
    docking = !docking;
    updateToolBar();
  });
}

void ChartsWidget::eventsMerged() {
  if (auto events = can->events(); events && !events->empty()) {
    event_range.first = (events->front()->mono_time / (double)1e9) - can->routeStartTime();
    event_range.second = (events->back()->mono_time / (double)1e9) - can->routeStartTime();
    updateDisplayRange();
  }
}

void ChartsWidget::updateDisplayRange() {
  auto prev_range = display_range;
  double current_sec = can->currentSec();
  if (current_sec < display_range.first || current_sec >= (display_range.second - 5)) {
    // reached the end, or seeked to a timestamp out of range.
    display_range.first = current_sec - 5;
  }
  display_range.first = std::max(display_range.first, event_range.first);
  display_range.second = std::min(display_range.first + settings.max_chart_x_range, event_range.second);
  if (prev_range != display_range) {
    QFutureSynchronizer<void> future_synchronizer;
    for (auto c : charts)
      future_synchronizer.addFuture(QtConcurrent::run(c, &ChartView::setEventsRange, display_range));
  }
}

void ChartsWidget::zoomIn(double min, double max) {
  zoomed_range = {min, max};
  is_zoomed = zoomed_range != display_range;
  updateToolBar();
  updateState();
  emit rangeChanged(min, max, is_zoomed);
}

void ChartsWidget::zoomReset() {
  zoomIn(display_range.first, display_range.second);
}

void ChartsWidget::updateState() {
  if (charts.isEmpty()) return;

  if (!is_zoomed) {
    updateDisplayRange();
  } else if (can->currentSec() < zoomed_range.first || can->currentSec() >= zoomed_range.second) {
    can->seekTo(zoomed_range.first);
  }

  const auto &range = is_zoomed ? zoomed_range : display_range;
  for (auto c : charts) {
    c->setDisplayRange(range.first, range.second);
    c->scene()->invalidate({}, QGraphicsScene::ForegroundLayer);
  }
}

void ChartsWidget::updateToolBar() {
  remove_all_btn->setEnabled(!charts.isEmpty());
  reset_zoom_btn->setEnabled(is_zoomed);
  range_label->setText(is_zoomed ? tr("%1 - %2").arg(zoomed_range.first, 0, 'f', 2).arg(zoomed_range.second, 0, 'f', 2) : "");
  title_label->setText(charts.size() > 0 ? tr("Charts (%1)").arg(charts.size()) : tr("Charts"));
  dock_btn->setText(docking ? "⬈" : "⬋");
  dock_btn->setToolTip(docking ? tr("Undock charts") : tr("Dock charts"));
}

ChartView *ChartsWidget::findChart(const QString &id, const Signal *sig) {
  for (auto c : charts)
    if (c->hasSeries(id, sig)) return c;
  return nullptr;
}

void ChartsWidget::showChart(const QString &id, const Signal *sig, bool show, bool merge) {
  setUpdatesEnabled(false);
  if (show) {
    ChartView *chart = merge && charts.size() > 0 ? charts.back() : nullptr;
    if (!chart) {
      chart = new ChartView(this);
      chart->setEventsRange(display_range);
      auto range = is_zoomed ? zoomed_range : display_range;
      chart->setDisplayRange(range.first, range.second);
      QObject::connect(chart, &ChartView::remove, [=]() { removeChart(chart); });
      QObject::connect(chart, &ChartView::zoomIn, this, &ChartsWidget::zoomIn);
      QObject::connect(chart, &ChartView::zoomReset, this, &ChartsWidget::zoomReset);
      QObject::connect(chart, &ChartView::seriesRemoved, this, &ChartsWidget::chartClosed);
      charts_layout->insertWidget(0, chart);
      charts.push_back(chart);
    }
    chart->addSeries(id, sig);
    emit chartOpened(id, sig);
  } else if (ChartView *chart = findChart(id, sig)) {
    chart->removeSeries(id, sig);
  }
  updateToolBar();
  setUpdatesEnabled(true);
}

void ChartsWidget::removeChart(ChartView *chart) {
  charts.removeOne(chart);
  chart->deleteLater();
  updateToolBar();
}

void ChartsWidget::removeAll() {
  for (auto c : charts.toVector())
    removeChart(c);
}

bool ChartsWidget::eventFilter(QObject *obj, QEvent *event) {
  if (obj != this && event->type() == QEvent::Close) {
    emit dock_btn->triggered();
    return true;
  }
  return false;
}

// ChartView

ChartView::ChartView(QWidget *parent) : QChartView(nullptr, parent) {
  QChart *chart = new QChart();
  chart->setBackgroundRoundness(0);
  axis_x = new QValueAxis(this);
  axis_y = new QValueAxis(this);
  chart->addAxis(axis_x, Qt::AlignBottom);
  chart->addAxis(axis_y, Qt::AlignLeft);
  chart->legend()->setShowToolTips(true);
  chart->layout()->setContentsMargins(0, 0, 0, 0);
  chart->setMargins(QMargins(20, 11, 11, 11));

  track_line = new QGraphicsLineItem(chart);
  track_line->setPen(QPen(Qt::darkGray, 1, Qt::DashLine));
  track_ellipse = new QGraphicsEllipseItem(chart);
  track_ellipse->setBrush(Qt::darkGray);
  value_text = new QGraphicsTextItem(chart);
  item_group = scene()->createItemGroup({track_line, track_ellipse, value_text});
  item_group->setZValue(chart->zValue() + 10);

  // title
  QToolButton *remove_btn = new QToolButton();
  remove_btn->setText("X");
  remove_btn->setAutoRaise(true);
  remove_btn->setToolTip(tr("Remove Chart"));
  close_btn_proxy = new QGraphicsProxyWidget(chart);
  close_btn_proxy->setWidget(remove_btn);
  close_btn_proxy->setZValue(chart->zValue() + 11);

  setChart(chart);
  setRenderHint(QPainter::Antialiasing);
  setRubberBand(QChartView::HorizontalRubberBand);
  updateFromSettings();

  QObject::connect(dbc(), &DBCManager::signalRemoved, this, &ChartView::signalRemoved);
  QObject::connect(dbc(), &DBCManager::signalUpdated, this, &ChartView::signalUpdated);
  QObject::connect(dbc(), &DBCManager::msgRemoved, this, &ChartView::msgRemoved);
  QObject::connect(dbc(), &DBCManager::msgUpdated, this, &ChartView::msgUpdated);
  QObject::connect(&settings, &Settings::changed, this, &ChartView::updateFromSettings);
  QObject::connect(remove_btn, &QToolButton::clicked, this, &ChartView::remove);
}

ChartView::~ChartView() {
  for (auto &s : sigs)
    emit seriesRemoved(s.msg_id, s.sig);
}

void ChartView::addSeries(const QString &msg_id, const Signal *sig) {
  QLineSeries *series = new QLineSeries(this);
  chart()->addSeries(series);
  series->attachAxis(axis_x);
  series->attachAxis(axis_y);
  auto [source, address] = DBCManager::parseId(msg_id);
  sigs.push_back({.msg_id = msg_id, .address = address, .source = source, .sig = sig, .series = series});
  updateTitle();
  updateSeries(sig);
}

void ChartView::removeSeries(const QString &msg_id, const Signal *sig) {
  auto it = std::find_if(sigs.begin(), sigs.end(), [&](auto &s) { return s.msg_id == msg_id && s.sig == sig; });
  if (it != sigs.end()) {
    it = removeSeries(it);
  }
}

bool ChartView::hasSeries(const QString &msg_id, const Signal *sig) const {
  auto it = std::find_if(sigs.begin(), sigs.end(), [&](auto &s) { return s.msg_id == msg_id && s.sig == sig; });
  return it != sigs.end();
}

QList<ChartView::SigItem>::iterator ChartView::removeSeries(const QList<ChartView::SigItem>::iterator &it) {
  chart()->removeSeries(it->series);
  it->series->deleteLater();
  emit seriesRemoved(it->msg_id, it->sig);

  auto ret = sigs.erase(it);
  if (!sigs.isEmpty()) {
    updateAxisY();
  } else {
    emit remove();
  }
  return ret;
}

void ChartView::signalUpdated(const Signal *sig) {
  auto it = std::find_if(sigs.begin(), sigs.end(), [=](auto &s) { return s.sig == sig; });
  if (it != sigs.end()) {
    updateTitle();
    // TODO: don't update series if only name changed.
    updateSeries(sig);
  }
}

void ChartView::signalRemoved(const Signal *sig) {
  for (auto it = sigs.begin(); it != sigs.end(); /**/) {
    it = (it->sig == sig) ? removeSeries(it) : ++it;
  }
}

void ChartView::msgUpdated(uint32_t address) {
  auto it = std::find_if(sigs.begin(), sigs.end(), [=](auto &s) { return s.address == address; });
  if (it != sigs.end())
    updateTitle();
}

void ChartView::msgRemoved(uint32_t address) {
  for (auto it = sigs.begin(); it != sigs.end(); /**/) {
    it = (it->address == address) ? removeSeries(it) : ++it;
  }
}

void ChartView::resizeEvent(QResizeEvent *event) {
  QChartView::resizeEvent(event);
  close_btn_proxy->setPos(event->size().width() - close_btn_proxy->size().width() - 11, 8);
}

void ChartView::updateTitle() {
  for (auto &s : sigs) {
    s.series->setName(QString("<b>%1</b> <font color=\"gray\">%2 %3</font>").arg(s.sig->name.c_str()).arg(msgName(s.msg_id)).arg(s.msg_id));
  }
}

void ChartView::updateFromSettings() {
  setFixedHeight(settings.chart_height);
  chart()->setTheme(settings.chart_theme == 0 ? QChart::ChartThemeLight : QChart::QChart::ChartThemeDark);
}

void ChartView::setEventsRange(const std::pair<double, double> &range) {
  if (range != events_range) {
    events_range = range;
    updateSeries();
  }
}

void ChartView::setDisplayRange(double min, double max) {
  if (min != axis_x->min() || max != axis_x->max()) {
    axis_x->setRange(min, max);
    updateAxisY();
  }
}

void ChartView::adjustChartMargins() {
  // TODO: Remove hardcoded aligned_pos
  const int aligned_pos = 60;
  if ((int)chart()->plotArea().left() != aligned_pos) {
    const float left_margin = chart()->margins().left() + aligned_pos - chart()->plotArea().left();
    chart()->setMargins(QMargins(left_margin, 11, 11, 11));
    scene()->invalidate({}, QGraphicsScene::ForegroundLayer);
  }
}

void ChartView::updateSeries(const Signal *sig) {
  auto events = can->events();
  if (!events || sigs.isEmpty()) return;

  for (auto &s : sigs) {
    if (!sig || s.sig == sig) {
      s.vals.clear();
      s.vals.reserve((events_range.second - events_range.first) * 1000);  // [n]seconds * 1000hz
      s.min_y = std::numeric_limits<double>::max();
      s.max_y = std::numeric_limits<double>::lowest();

      double route_start_time = can->routeStartTime();
      Event begin_event(cereal::Event::Which::INIT_DATA, (route_start_time + events_range.first) * 1e9);
      auto begin = std::lower_bound(events->begin(), events->end(), &begin_event, Event::lessThan());
      double end_ns = (route_start_time + events_range.second) * 1e9;

      for (auto it = begin; it != events->end() && (*it)->mono_time <= end_ns; ++it) {
        if ((*it)->which == cereal::Event::Which::CAN) {
          for (const auto &c : (*it)->event.getCan()) {
            if (s.source == c.getSrc() && s.address == c.getAddress()) {
              auto dat = c.getDat();
              double value = get_raw_value((uint8_t *)dat.begin(), dat.size(), *s.sig);
              double ts = ((*it)->mono_time / (double)1e9) - route_start_time;  // seconds
              s.vals.push_back({ts, value});

              if (value < s.min_y) s.min_y = value;
              if (value > s.max_y) s.max_y = value;
            }
          }
        }
      }
      s.series->replace(s.vals);
    }
  }
  updateAxisY();
}

// auto zoom on yaxis
void ChartView::updateAxisY() {
  if (sigs.isEmpty()) return;

  double min_y = std::numeric_limits<double>::max();
  double max_y = std::numeric_limits<double>::lowest();
  if (events_range == std::pair{axis_x->min(), axis_x->max()}) {
    for (auto &s : sigs) {
      if (s.min_y < min_y) min_y = s.min_y;
      if (s.max_y > max_y) max_y = s.max_y;
    }
  } else {
    for (auto &s : sigs) {
      auto begin = std::lower_bound(s.vals.begin(), s.vals.end(), axis_x->min(), [](auto &p, double x) { return p.x() < x; });
      for (auto it = begin; it != s.vals.end() && it->x() <= axis_x->max(); ++it) {
        if (it->y() < min_y) min_y = it->y();
        if (it->y() > max_y) max_y = it->y();
      }
    }
  }

  if (min_y == std::numeric_limits<double>::max()) min_y = 0;
  if (max_y == std::numeric_limits<double>::lowest()) max_y = 0;
  if (max_y == min_y) {
    axis_y->setRange(min_y - 1, max_y + 1);
  } else {
    double range = max_y - min_y;
    axis_y->setRange(min_y - range * 0.05, max_y + range * 0.05);
    axis_y->applyNiceNumbers();
  }

  QTimer::singleShot(0, this, &ChartView::adjustChartMargins);
}

void ChartView::leaveEvent(QEvent *event) {
  item_group->setVisible(false);
  QChartView::leaveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event) {
  auto rubber = findChild<QRubberBand *>();
  if (event->button() == Qt::LeftButton && rubber && rubber->isVisible()) {
    rubber->hide();
    QRectF rect = rubber->geometry().normalized();
    double min = chart()->mapToValue(rect.topLeft()).x();
    double max = chart()->mapToValue(rect.bottomRight()).x();
    if (rubber->width() <= 0) {
      // no rubber dragged, seek to mouse position
      can->seekTo(min);
    } else if ((max - min) >= 0.5) {
      // zoom in if selected range is greater than 0.5s
      emit zoomIn(min, max);
    }
    event->accept();
  } else if (event->button() == Qt::RightButton) {
    emit zoomReset();
    event->accept();
  } else {
    QGraphicsView::mouseReleaseEvent(event);
  }
}

void ChartView::mouseMoveEvent(QMouseEvent *ev) {
  auto rubber = findChild<QRubberBand *>();
  bool is_zooming = rubber && rubber->isVisible();
  const auto plot_area = chart()->plotArea();

  if (!is_zooming && plot_area.contains(ev->pos())) {
    QStringList text_list;
    QPointF pos = {};
    const double sec = chart()->mapToValue(ev->pos()).x();
    for (auto &s : sigs) {
      QString value = "--";
      // use reverse iterator to find last item <= sec.
      auto it = std::lower_bound(s.vals.rbegin(), s.vals.rend(), sec, [](auto &p, double x) { return p.x() > x; });
      if (it != s.vals.rend() && it->x() >= axis_x->min()) {
        value = QString::number(it->y());
        auto value_pos = chart()->mapToPosition(*it);
        if (value_pos.x() > pos.x()) pos = value_pos;
      }
      text_list.push_back(QString("&nbsp;%1 : %2&nbsp;").arg(sigs.size() > 1 ? s.sig->name.c_str() : "Value").arg(value));
    }
    if (pos.x() == 0) pos = ev->pos();

    QString time = QString::number(chart()->mapToValue(pos).x(), 'f', 3);
    value_text->setHtml(QString("<div style=\"background-color: darkGray;color: white;\">&nbsp;Time: %1 &nbsp;<br />%2</div>")
                            .arg(time).arg(text_list.join("<br />")));

    QRectF text_rect = value_text->boundingRect();
    int text_x = pos.x() + 8;
    if ((text_x + text_rect.width()) > plot_area.right()) {
      text_x = pos.x() - text_rect.width() - 8;
    }
    value_text->setPos(text_x, pos.y() - text_rect.height() / 2);
    track_line->setLine(pos.x(), plot_area.top(), pos.x(), plot_area.bottom());
    track_ellipse->setRect(pos.x() - 5, pos.y() - 5, 10, 10);
    item_group->setVisible(true);
  } else {
    item_group->setVisible(false);
  }
  QChartView::mouseMoveEvent(ev);
}

void ChartView::drawForeground(QPainter *painter, const QRectF &rect) {
  qreal x = chart()->mapToPosition(QPointF{can->currentSec(), 0}).x();
  painter->setPen(QPen(chart()->titleBrush().color(), 2));
  painter->drawLine(QPointF{x, chart()->plotArea().top() - 2}, QPointF{x, chart()->plotArea().bottom() + 2});
}
