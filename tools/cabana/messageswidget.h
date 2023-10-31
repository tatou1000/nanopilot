#pragma once

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QToolBar>
#include <QTreeView>

#include "tools/cabana/dbc/dbcmanager.h"
#include "tools/cabana/streams/abstractstream.h"

class MessageListModel : public QAbstractTableModel {
Q_OBJECT

public:
  enum Column {
    NAME = 0,
    SOURCE,
    ADDRESS,
    NODE,
    FREQ,
    COUNT,
    DATA,
  };

  MessageListModel(QObject *parent) : QAbstractTableModel(parent) {}
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override { return Column::DATA + 1; }
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override { return items_.size(); }
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
  void setFilterStrings(const QMap<int, QString> &filters);
  void msgsReceived(const std::set<MessageId> *new_msgs, bool has_new_ids);
  void filterAndSort();
  void dbcModified();

  struct Item {
    MessageId id;
    QString name;
    QString node;
    bool operator==(const Item &other) const {
      return id == other.id && name == other.name && node == other.node;
    }
  };
  std::vector<Item> items_;

private:
  void sortItems(std::vector<MessageListModel::Item> &items);
  bool match(const MessageListModel::Item &id);

  QMap<int, QString> filters_;
  std::set<MessageId> dbc_messages_;
  int sort_column = 0;
  Qt::SortOrder sort_order = Qt::AscendingOrder;
};

class MessageView : public QTreeView {
  Q_OBJECT
public:
  MessageView(QWidget *parent) : QTreeView(parent) {}
  void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override {}
  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
  void updateBytesSectionSize();
};

class MessageViewHeader : public QHeaderView {
  // https://stackoverflow.com/a/44346317
  Q_OBJECT
public:
  MessageViewHeader(QWidget *parent);
  void updateHeaderPositions();
  void updateGeometries() override;
  QSize sizeHint() const override;

signals:
  void filtersUpdated(const QMap<int, QString> &filters);

private:
  void updateFilters();

  QMap<int, QLineEdit *> editors;
};

class MessagesWidget : public QWidget {
  Q_OBJECT

public:
  MessagesWidget(QWidget *parent);
  void selectMessage(const MessageId &message_id);
  QByteArray saveHeaderState() const { return view->header()->saveState(); }
  bool restoreHeaderState(const QByteArray &state) const { return view->header()->restoreState(state); }
  void suppressHighlighted();

public slots:
  void dbcModified();

signals:
  void msgSelectionChanged(const MessageId &message_id);

protected:
  QToolBar *createToolBar();
  void headerContextMenuEvent(const QPoint &pos);
  void menuAboutToShow();
  void setMultiLineBytes(bool multi);

  MessageView *view;
  MessageViewHeader *header;
  MessageBytesDelegate *delegate;
  std::optional<MessageId> current_msg_id;
  MessageListModel *model;
  QPushButton *suppress_add;
  QPushButton *suppress_clear;
  QLabel *num_msg_label;
  QMenu *menu;
};
