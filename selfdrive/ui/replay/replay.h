#pragma once

#include <QThread>

#include <capnp/dynamic.h>
#include "cereal/visionipc/visionipc_server.h"
#include "selfdrive/ui/replay/camera.h"
#include "selfdrive/ui/replay/route.h"

constexpr int FORWARD_SEGS = 2;
constexpr int BACKWARD_SEGS = 2;

class Replay : public QObject {
  Q_OBJECT

public:
  Replay(QString route, QStringList allow, QStringList block, SubMaster *sm = nullptr, bool dcam = false, bool ecam = false, QObject *parent = 0);
  ~Replay();
  bool load();
  void start(int seconds = 0);
  void seekTo(int seconds, bool relative = false);
  void relativeSeek(int seconds) { seekTo(seconds, true); }
  void pause(bool pause);
  bool isPaused() const { return paused_; }

signals:
 void segmentChanged();

protected slots:
  void queueSegment();

protected:
  void stream();
  void setCurrentSegment(int n);
  void mergeSegments(int begin_idx, int end_idx);
  void updateEvents(const std::function<bool()>& lambda);
  void publishFrame(const Event *e);

  QThread *thread;

  // logs
  std::mutex stream_lock_;
  std::condition_variable stream_cv_;
  std::atomic<bool> updating_events_ = false;
  std::atomic<int> current_segment_ = -1;
  std::vector<std::unique_ptr<Segment>> segments_;
  // the following variables must be protected with stream_lock_
  bool exit_ = false;
  bool paused_ = false;
  bool events_updated_ = false;
  uint64_t route_start_ts_ = 0;
  uint64_t cur_mono_time_ = 0;
  std::vector<Event *> *events_ = nullptr;
  std::vector<int> segments_merged_;

  // messaging
  SubMaster *sm;
  PubMaster *pm;
  std::vector<const char*> sockets_;
  std::unique_ptr<Route> route_;
  bool load_dcam = false, load_ecam = false;
  std::unique_ptr<CameraServer> camera_server_;
};
