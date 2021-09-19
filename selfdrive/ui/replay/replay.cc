#include "selfdrive/ui/replay/replay.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "cereal/services.h"
#include "selfdrive/camerad/cameras/camera_common.h"
#include "selfdrive/common/timing.h"
#include "selfdrive/hardware/hw.h"

Replay::Replay(QString route, QStringList allow, QStringList block, SubMaster *sm_, QObject *parent) : sm(sm_), QObject(parent) {
  std::vector<const char*> s;
  for (const auto &it : services) {
    if ((allow.size() == 0 || allow.contains(it.name)) &&
        !block.contains(it.name)) {
      s.push_back(it.name);
      socks.append(std::string(it.name));
    }
  }
  qDebug() << "services " << s;

  if (sm == nullptr) {
    pm = new PubMaster(s);
  }

  const QString url = CommaApi::BASE_URL + "/v1/route/" + route + "/files";
  http = new HttpRequest(this, !Hardware::PC());
  QObject::connect(http, &HttpRequest::receivedResponse, this, &Replay::parseResponse);
  http->sendRequest(url);
}

void Replay::parseResponse(const QString &response) {
  QJsonDocument doc = QJsonDocument::fromJson(response.trimmed().toUtf8());
  if (doc.isNull()) {
    qDebug() << "JSON Parse failed";
    return;
  }

  camera_paths = doc["cameras"].toArray();
  log_paths = doc["logs"].toArray();

  seekTo(0);
}

void Replay::addSegment(int n) {
  assert((n >= 0) && (n < log_paths.size()) && (n < camera_paths.size()));
  if (lrs.find(n) != lrs.end()) {
    return;
  }

  lrs[n] = new LogReader(log_paths.at(n).toString());
  // this is a queued connection, mergeEvents is executed in the main thread.
  QObject::connect(lrs[n], &LogReader::finished, this, &Replay::mergeEvents);

  frs[n] = new FrameReader(qPrintable(camera_paths.at(n).toString()));
  QThread * t = QThread::create([=]() { frs[n]->process(); });
  QObject::connect(t, &QThread::finished, t, &QThread::deleteLater);
  t->start();
}

void Replay::mergeEvents() {
  const int start_idx = std::max(current_segment - BACKWARD_SEGS, 0);
  const int end_idx = std::min(current_segment + FORWARD_SEGS, log_paths.size());

  // merge logs
  QMultiMap<uint64_t, Event *> *new_events = new QMultiMap<uint64_t, Event *>();
  std::unordered_map<uint32_t, EncodeIdx> *new_eidx = new std::unordered_map<uint32_t, EncodeIdx>[MAX_CAMERAS];
  for (int i = start_idx; i <= end_idx; ++i) {
    if (auto it = lrs.find(i); it != lrs.end()) {
      *new_events += (*it)->events;
      for (CameraType cam_type : ALL_CAMERAS) {
        new_eidx[cam_type].insert((*it)->eidx[cam_type].begin(), (*it)->eidx[cam_type].end());
      }
    }
  }

  // update logs
  updating_events = true; // set updating_events to true to force stream thread relase the lock
  lock.lock();
  auto prev_events = std::exchange(events, new_events);
  auto prev_eidx = std::exchange(eidx, new_eidx);
  updating_events = false;
  lock.unlock();

  // free logs
  delete prev_events;
  delete[] prev_eidx;
  for (int i = 0; i < log_paths.size(); i++) {
    if (i < start_idx || i > end_idx) {
      delete lrs.take(i);
      delete frs.take(i);
    }
  }
}

void Replay::start(){
  thread = new QThread;
  QObject::connect(thread, &QThread::started, [=](){
    stream();
  });
  thread->start();

  queue_thread = new QThread;
  QObject::connect(queue_thread, &QThread::started, [=](){
    segmentQueueThread();
  });
  queue_thread->start();
}

void Replay::seekTo(int seconds) {
  updating_events = true;

  std::unique_lock lk(lock);
  seconds = std::clamp(seconds, 0, log_paths.size() * 60);
  qInfo() << "seeking to " << seconds;
  seek_ts = seconds;
  current_segment = seconds / 60;
  updating_events = false;
}

void Replay::relativeSeek(int seconds) {
  if (current_ts > 0) {
    seekTo(current_ts + seconds);
  }
}

void Replay::segmentQueueThread() {
  // maintain the segment window
  while (true) {
    int start_idx = std::max(current_segment - BACKWARD_SEGS, 0);
    int end_idx = std::min(current_segment + FORWARD_SEGS, log_paths.size());
    for (int i = 0; i < log_paths.size(); i++) {
      if (i >= start_idx && i <= end_idx) {
        addSegment(i);
      }
    }
    QThread::msleep(100);
  }
}

void Replay::stream() {
  QElapsedTimer timer;
  timer.start();

  route_start_ts = 0;
  uint64_t cur_mono_time = 0;
  while (true) {
    std::unique_lock lk(lock);

    if (!events || events->size() == 0) {
      lk.unlock();
      qDebug() << "waiting for events";
      QThread::msleep(100);
      continue;
    }

    // TODO: use initData's logMonoTime
    if (route_start_ts == 0) {
      route_start_ts = events->firstKey();
    }

    uint64_t t0 = seek_ts != -1 ? route_start_ts + (seek_ts * 1e9) : cur_mono_time;
    seek_ts = -1;
    qDebug() << "unlogging at" << int((t0 - route_start_ts) / 1e9);
    uint64_t t0r = timer.nsecsElapsed();

    for (auto eit = events->lowerBound(t0); !updating_events && eit != events->end(); ++eit) {
      cereal::Event::Reader e = (*eit)->event;
      cur_mono_time = (*eit)->mono_time;
      current_segment = (cur_mono_time - route_start_ts) / 1e9 / 60;
      std::string type;
      KJ_IF_MAYBE(e_, static_cast<capnp::DynamicStruct::Reader>(e).which()) {
        type = e_->getProto().getName();
      }

      current_ts = std::max(cur_mono_time - route_start_ts, (uint64_t)0) / 1e9;

      if (socks.contains(type)) {
        float timestamp = (cur_mono_time - route_start_ts)/1e9;
        if (std::abs(timestamp - last_print) > 5.0) {
          last_print = timestamp;
          qInfo() << "at " << int(last_print) << "s";
        }

        // keep time
        long etime = cur_mono_time-t0;
        long rtime = timer.nsecsElapsed() - t0r;
        long us_behind = ((etime-rtime)*1e-3)+0.5;
        if (us_behind > 0 && us_behind < 1e6) {
          QThread::usleep(us_behind);
          //qDebug() << "sleeping" << us_behind << etime << timer.nsecsElapsed();
        }

        // publish frame
        // TODO: publish all frames
        if (type == "roadCameraState") {
          auto fr = e.getRoadCameraState();

          auto it_ = eidx[RoadCam].find(fr.getFrameId());
          if (it_ != eidx[RoadCam].end()) {
            EncodeIdx &e = it_->second;
            if (frs.find(e.segmentNum) != frs.end()) {
              auto frm = frs[e.segmentNum];
              if (vipc_server == nullptr) {
                cl_device_id device_id = cl_get_device_id(CL_DEVICE_TYPE_DEFAULT);
                cl_context context = CL_CHECK_ERR(clCreateContext(NULL, 1, &device_id, NULL, NULL, &err));

                vipc_server = new VisionIpcServer("camerad", device_id, context);
                vipc_server->create_buffers(VisionStreamType::VISION_STREAM_RGB_BACK, UI_BUF_COUNT,
                                            true, frm->width, frm->height);
                vipc_server->start_listener();
              }

              uint8_t *dat = frm->get(e.frameEncodeId);
              if (dat) {
                VisionIpcBufExtra extra = {};
                VisionBuf *buf = vipc_server->get_buffer(VisionStreamType::VISION_STREAM_RGB_BACK);
                memcpy(buf->addr, dat, frm->getRGBSize());
                vipc_server->send(buf, &extra, false);
              }
            }
          }
        }

        // publish msg
        if (sm == nullptr) {
          auto bytes = (*eit)->bytes();
          pm->send(type.c_str(), (capnp::byte *)bytes.begin(), bytes.size());
        } else {
          std::vector<std::pair<std::string, cereal::Event::Reader>> messages;
          messages.push_back({type, e});
          sm->update_msgs(nanos_since_boot(), messages);
        }
      }
    }
    lk.unlock();
    usleep(0);
  }
}
