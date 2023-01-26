#include "tools/cabana/historylog.h"

#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

// HistoryLogModel

QVariant HistoryLogModel::data(const QModelIndex &index, int role) const {
  const bool show_signals = display_signals_mode && sigs.size() > 0;
  const auto &m = messages[index.row()];
  if (role == Qt::DisplayRole) {
    if (index.column() == 0) {
      return QString::number((m.mono_time / (double)1e9) - can->routeStartTime(), 'f', 2);
    }
    return show_signals ? QString::number(m.sig_values[index.column() - 1]) : toHex(m.data);
  } else if (role == Qt::UserRole && index.column() == 1 && !show_signals) {
    return HexColors::toVariantList(m.colors);
  }
  return {};
}

void HistoryLogModel::setMessage(const QString &message_id) {
  msg_id = message_id;
  sigs.clear();
  if (auto dbc_msg = dbc()->msg(msg_id)) {
    sigs = dbc_msg->getSignals();
  }
  filter_cmp = nullptr;
  refresh();
}

void HistoryLogModel::refresh() {
  beginResetModel();
  last_fetch_time = 0;
  messages.clear();
  hex_colors.clear();
  updateState();
  endResetModel();
}

QVariant HistoryLogModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal) {
    const bool show_signals = display_signals_mode && !sigs.empty();
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
      if (section == 0) {
        return "Time";
      }
      return show_signals ? QString::fromStdString(sigs[section - 1]->name).replace('_', ' ') : "Data";
    } else if (role == Qt::BackgroundRole && section > 0 && show_signals) {
      return QBrush(QColor(getColor(section - 1)));
    }
  }
  return {};
}

void HistoryLogModel::setDynamicMode(int state) {
  dynamic_mode = state != 0;
  refresh();
}

void HistoryLogModel::setDisplayType(int type) {
  display_signals_mode = type == 0;
  refresh();
}

void HistoryLogModel::segmentsMerged() {
  if (!dynamic_mode) {
    has_more_data = true;
  }
}

void HistoryLogModel::setFilter(int sig_idx, const QString &value, std::function<bool(double, double)> cmp) {
  filter_sig_idx = sig_idx;
  filter_value = value.toDouble();
  filter_cmp = value.isEmpty() ? nullptr : cmp;
  refresh();
}

void HistoryLogModel::updateState() {
  if (!msg_id.isEmpty()) {
    uint64_t current_time = (can->currentSec() + can->routeStartTime()) * 1e9;
    auto new_msgs = dynamic_mode ? fetchData(current_time, last_fetch_time) : fetchData(0);
    if ((has_more_data = !new_msgs.empty())) {
      beginInsertRows({}, 0, new_msgs.size() - 1);
      messages.insert(messages.begin(), std::move_iterator(new_msgs.begin()), std::move_iterator(new_msgs.end()));
      updateColors();
      endInsertRows();
    }
    last_fetch_time = current_time;
  }
}

void HistoryLogModel::fetchMore(const QModelIndex &parent) {
  if (!messages.empty()) {
    auto new_msgs = fetchData(messages.back().mono_time);
    if ((has_more_data = !new_msgs.empty())) {
      beginInsertRows({}, messages.size(), messages.size() + new_msgs.size() - 1);
      messages.insert(messages.end(), std::move_iterator(new_msgs.begin()), std::move_iterator(new_msgs.end()));
      if (!dynamic_mode) {
        updateColors();
      }
      endInsertRows();
    }
  }
}

void HistoryLogModel::updateColors() {
  if (!display_signals_mode || sigs.empty()) {
    const auto freq = can->lastMessage(msg_id).freq;
    if (dynamic_mode) {
      for (auto it = messages.rbegin(); it != messages.rend(); ++it) {
        it->colors = hex_colors.compute(it->data, it->mono_time / (double)1e9, freq);
      }
    } else {
      for (auto it = messages.begin(); it != messages.end(); ++it) {
        it->colors = hex_colors.compute(it->data, it->mono_time / (double)1e9, freq);
      }
    }
  }
}

template <class InputIt>
std::deque<HistoryLogModel::Message> HistoryLogModel::fetchData(InputIt first, InputIt last, uint64_t min_time) {
  std::deque<HistoryLogModel::Message> msgs;
  const auto [src, address] = DBCManager::parseId(msg_id);
  QVector<double> values(sigs.size());
  for (auto it = first; it != last && (*it)->mono_time > min_time; ++it) {
    if ((*it)->which == cereal::Event::Which::CAN) {
      for (const auto &c : (*it)->event.getCan()) {
        if (src == c.getSrc() && address == c.getAddress()) {
          const auto dat = c.getDat();
          for (int i = 0; i < sigs.size(); ++i) {
            values[i] = get_raw_value((uint8_t *)dat.begin(), dat.size(), *(sigs[i]));
          }
          if (!filter_cmp || filter_cmp(values[filter_sig_idx], filter_value)) {
            auto &m = msgs.emplace_back();
            m.mono_time = (*it)->mono_time;
            m.data = QByteArray((char *)dat.begin(), dat.size());
            m.sig_values = values;
            if (msgs.size() >= batch_size && min_time == 0)
              return msgs;
          }
        }
      }
    }
  }
  return msgs;
}

template std::deque<HistoryLogModel::Message> HistoryLogModel::fetchData<>(std::vector<const Event*>::iterator first, std::vector<const Event*>::iterator last, uint64_t min_time);
template std::deque<HistoryLogModel::Message> HistoryLogModel::fetchData<>(std::vector<const Event*>::reverse_iterator first, std::vector<const Event*>::reverse_iterator last, uint64_t min_time);

std::deque<HistoryLogModel::Message> HistoryLogModel::fetchData(uint64_t from_time, uint64_t min_time) {
  auto events = can->events();
  if (dynamic_mode) {
    auto it = std::upper_bound(events->rbegin(), events->rend(), from_time, [=](uint64_t ts, auto &e) {
      return e->mono_time < ts;
    });
    return fetchData(it, events->rend(), min_time);
  } else {
    assert(min_time == 0);
    auto it = std::upper_bound(events->begin(), events->end(), from_time, [=](uint64_t ts, auto &e) {
      return ts < e->mono_time;
    });
    return fetchData(it, events->end(), 0);
  }
}

// HeaderView

QSize HeaderView::sectionSizeFromContents(int logicalIndex) const {
  int default_size = qMax(100, rect().width() / model()->columnCount());
  const QString text = model()->headerData(logicalIndex, this->orientation(), Qt::DisplayRole).toString();
  const QRect rect = fontMetrics().boundingRect({0, 0, default_size, 2000}, defaultAlignment(), text);
  QSize size = rect.size() + QSize{10, 6};
  return {qMax(size.width(), default_size), size.height()};
}

void HeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const {
  auto bg_role = model()->headerData(logicalIndex, Qt::Horizontal, Qt::BackgroundRole);
  if (bg_role.isValid()) {
    painter->fillRect(rect, bg_role.value<QBrush>());
  }
  QString text = model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString();
  painter->drawText(rect.adjusted(5, 3, -5, -3), defaultAlignment(), text);
}

// LogsWidget

LogsWidget::LogsWidget(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);

  QHBoxLayout *h = new QHBoxLayout();
  filters_widget = new QWidget(this);
  QHBoxLayout *filter_layout = new QHBoxLayout(filters_widget);
  filter_layout->setContentsMargins(0, 0, 0, 0);
  filter_layout->addWidget(display_type_cb = new QComboBox(this));
  filter_layout->addWidget(signals_cb = new QComboBox(this));
  filter_layout->addWidget(comp_box = new QComboBox(this));
  filter_layout->addWidget(value_edit = new QLineEdit(this));
  h->addWidget(filters_widget);
  h->addStretch(0);
  h->addWidget(dynamic_mode = new QCheckBox(tr("Dynamic")), 0, Qt::AlignRight);

  display_type_cb->addItems({"Signal Value", "Hex Value"});
  comp_box->addItems({">", "=", "!=", "<"});
  value_edit->setClearButtonEnabled(true);
  value_edit->setValidator(new QDoubleValidator(-500000, 500000, 6, this));
  dynamic_mode->setChecked(true);
  dynamic_mode->setEnabled(!can->liveStreaming());

  main_layout->addLayout(h);
  main_layout->addWidget(logs = new QTableView(this));
  logs->setModel(model = new HistoryLogModel(this));
  logs->setItemDelegateForColumn(1, new MessageBytesDelegate(this));
  logs->setHorizontalHeader(new HeaderView(Qt::Horizontal, this));
  logs->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | (Qt::Alignment)Qt::TextWordWrap);
  logs->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  logs->verticalHeader()->setVisible(false);

  QObject::connect(display_type_cb, SIGNAL(activated(int)), model, SLOT(setDisplayType(int)));
  QObject::connect(dynamic_mode, &QCheckBox::stateChanged, model, &HistoryLogModel::setDynamicMode);
  QObject::connect(signals_cb, SIGNAL(activated(int)), this, SLOT(setFilter()));
  QObject::connect(comp_box, SIGNAL(activated(int)), this, SLOT(setFilter()));
  QObject::connect(value_edit, &QLineEdit::textChanged, this, &LogsWidget::setFilter);
  QObject::connect(can, &AbstractStream::seekedTo, model, &HistoryLogModel::refresh);
  QObject::connect(can, &AbstractStream::eventsMerged, model, &HistoryLogModel::segmentsMerged);
}

void LogsWidget::setMessage(const QString &message_id) {
  model->setMessage(message_id);
  bool has_signal = model->sigs.size();
  if (has_signal) {
    signals_cb->clear();
    for (auto s : model->sigs) {
      signals_cb->addItem(s->name.c_str());
    }
  }
  value_edit->clear();
  comp_box->setCurrentIndex(0);
  filters_widget->setVisible(has_signal);
}

void LogsWidget::setFilter() {
  if (value_edit->text().isEmpty() && !value_edit->isModified()) return;

  std::function<bool(double, double)> cmp = nullptr;
  switch (comp_box->currentIndex()) {
    case 0: cmp = std::greater<double>{}; break;
    case 1: cmp = std::equal_to<double>{}; break;
    case 2: cmp = [](double l, double r) { return l != r; }; break; // not equal
    case 3: cmp = std::less<double>{}; break;
  }
  model->setFilter(signals_cb->currentIndex(), value_edit->text(), cmp);
}
