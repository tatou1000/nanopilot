#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common/swaglog.h"

#include <cassert>
#include <limits>
#include <mutex>
#include <string>

#include <zmq.h>
#include <stdarg.h>
#include "nlohmann/json.hpp"
#include "common/version.h"
#include "system/hardware/hw.h"

class SwaglogState {
public:
  SwaglogState() {
    zctx = zmq_ctx_new();
    sock = zmq_socket(zctx, ZMQ_PUSH);

    // Timeout on shutdown for messages to be received by the logging process
    int timeout = 100;
    zmq_setsockopt(sock, ZMQ_LINGER, &timeout, sizeof(timeout));
    zmq_connect(sock, Path::swaglog_ipc().c_str());

    print_level = CLOUDLOG_WARNING;
    if (const char* print_lvl = getenv("LOGPRINT")) {
      if (strcmp(print_lvl, "debug") == 0) {
        print_level = CLOUDLOG_DEBUG;
      } else if (strcmp(print_lvl, "info") == 0) {
        print_level = CLOUDLOG_INFO;
      } else if (strcmp(print_lvl, "warning") == 0) {
        print_level = CLOUDLOG_WARNING;
      }
    }

    ctx_j = nlohmann::json();
    if (char* dongle_id = getenv("DONGLE_ID")) {
      ctx_j["dongle_id"] = dongle_id;
    }
    if (char* daemon_name = getenv("MANAGER_DAEMON")) {
      ctx_j["daemon"] = daemon_name;
    }
    ctx_j["version"] = COMMA_VERSION;
    ctx_j["dirty"] = !getenv("CLEAN");
    ctx_j["device"] = Hardware::get_name();
  }

  ~SwaglogState() {
    zmq_close(sock);
    zmq_ctx_destroy(zctx);
  }

  void log(int levelnum, const char* filename, int lineno, const char* func, const char* msg, const std::string& log_s) {
    std::lock_guard lk(lock);
    if (levelnum >= print_level) {
      printf("%s: %s\n", filename, msg);
    }
    zmq_send(sock, log_s.data(), log_s.length(), ZMQ_NOBLOCK);
  }

  std::mutex lock;
  void* zctx = nullptr;
  void* sock = nullptr;
  int print_level;
  nlohmann::json ctx_j;
};

bool LOG_TIMESTAMPS = getenv("LOG_TIMESTAMPS");
uint32_t NO_FRAME_ID = std::numeric_limits<uint32_t>::max();

static void cloudlog_common(int levelnum, const char* filename, int lineno, const char* func,
                            char* msg_buf, const nlohmann::json &msg_j={}) {
  static SwaglogState s;

  nlohmann::json log_j = nlohmann::json {
    {"ctx", s.ctx_j},
    {"levelnum", levelnum},
    {"filename", filename},
    {"lineno", lineno},
    {"funcname", func},
    {"created", seconds_since_epoch()}
  };
  if (msg_j.empty()) {
    log_j["msg"] = msg_buf;
  } else {
    log_j["msg"] = msg_j;
  }

  std::string log_s;
  log_s += (char)levelnum;
  log_s += log_j.dump();
  s.log(levelnum, filename, lineno, func, msg_buf, log_s);

  free(msg_buf);
}

void cloudlog_e(int levelnum, const char* filename, int lineno, const char* func,
                const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* msg_buf = nullptr;
  int ret = vasprintf(&msg_buf, fmt, args);
  va_end(args);
  if (ret <= 0 || !msg_buf) return;
  cloudlog_common(levelnum, filename, lineno, func, msg_buf);
}

void cloudlog_t_common(int levelnum, const char* filename, int lineno, const char* func,
                       uint32_t frame_id, const char* fmt, va_list args) {
  if (!LOG_TIMESTAMPS) return;
  char* msg_buf = nullptr;
  int ret = vasprintf(&msg_buf, fmt, args);
  if (ret <= 0 || !msg_buf) return;
  nlohmann::json tspt_j = nlohmann::json{
    {"event", msg_buf},
    {"time", std::to_string(nanos_since_boot())}
  };
  if (frame_id < NO_FRAME_ID) {
    tspt_j["frame_id"] = std::to_string(frame_id);
  }
  tspt_j = nlohmann::json{{"timestamp", tspt_j}};
  cloudlog_common(levelnum, filename, lineno, func, msg_buf, tspt_j);
}


void cloudlog_te(int levelnum, const char* filename, int lineno, const char* func,
                 const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  cloudlog_t_common(levelnum, filename, lineno, func, NO_FRAME_ID, fmt, args);
  va_end(args);
}
void cloudlog_te(int levelnum, const char* filename, int lineno, const char* func,
                 uint32_t frame_id, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  cloudlog_t_common(levelnum, filename, lineno, func, frame_id, fmt, args);
  va_end(args);
}
