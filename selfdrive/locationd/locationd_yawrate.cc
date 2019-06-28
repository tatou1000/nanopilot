#include <iostream>
#include <csignal>
#include <cmath>

#include <czmq.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <eigen3/Eigen/Dense>
#include "json11.hpp"

#include "cereal/gen/cpp/log.capnp.h"
#include "common/swaglog.h"
#include "common/messaging.h"
#include "common/params.h"
#include "common/timing.h"
#include "params_learner.h"

const int num_polls = 3;

class Localizer
{
  Eigen::Matrix2d A;
  Eigen::Matrix2d I;
  Eigen::Matrix2d Q;
  Eigen::Matrix2d P;
  Eigen::Matrix<double, 1, 2> C_posenet;
  Eigen::Matrix<double, 1, 2> C_gyro;

  double R_gyro;

  void update_state(const Eigen::Matrix<double, 1, 2> &C, const double R, double current_time, double meas) {
    double dt = current_time - prev_update_time;
    prev_update_time = current_time;
    if (dt < 1.0e-9) {
      return;
    }

    // x = A * x;
    // P = A * P * A.transpose() + dt * Q;
    // Simplify because A is unity
    P = P + dt * Q;

    double y = meas - C * x;
    double S = R + C * P * C.transpose();
    Eigen::Vector2d K = P * C.transpose() * (1.0 / S);
    x = x + K * y;
    P = (I - K * C) * P;
  }

  void handle_sensor_events(capnp::List<cereal::SensorEventData>::Reader sensor_events, double current_time) {
    for (cereal::SensorEventData::Reader sensor_event : sensor_events){
      if (sensor_event.getType() == 4) {
        sensor_data_time = current_time;

        double meas = -sensor_event.getGyro().getV()[0];
        update_state(C_gyro, R_gyro, current_time, meas);
      }
    }

  }

  void handle_camera_odometry(cereal::CameraOdometry::Reader camera_odometry, double current_time) {
    double R = 250.0 * pow(camera_odometry.getRotStd()[2], 2);
    double meas = camera_odometry.getRot()[2];
    update_state(C_posenet, R, current_time, meas);
  }

  void handle_controls_state(cereal::ControlsState::Reader controls_state, double current_time) {
    steering_angle = controls_state.getAngleSteers() * DEGREES_TO_RADIANS;
    car_speed = controls_state.getVEgo();
    controls_state_time = current_time;
  }


public:
  Eigen::Vector2d x;
  double steering_angle = 0;
  double car_speed = 0;
  double prev_update_time = -1;
  double controls_state_time = -1;
  double sensor_data_time = -1;

  Localizer() {
    A << 1, 0, 0, 1;
    I << 1, 0, 0, 1;

    Q << pow(0.1, 2.0), 0, 0, pow(0.005 / 100.0, 2.0);
    P << pow(1.0, 2.0), 0, 0, pow(0.05, 2.0);

    C_posenet << 1, 0;
    C_gyro << 1, 1;
    x << 0, 0;

    R_gyro = pow(0.05, 2.0);
  }

  cereal::Event::Which handle_log(const unsigned char* msg_dat, size_t msg_size) {
    const kj::ArrayPtr<const capnp::word> view((const capnp::word*)msg_dat, msg_size);
    capnp::FlatArrayMessageReader msg(view);
    cereal::Event::Reader event = msg.getRoot<cereal::Event>();
    double current_time = event.getLogMonoTime() / 1.0e9;

    if (prev_update_time < 0) {
      prev_update_time = current_time;
    }

    auto type = event.which();
    switch(type) {
    case cereal::Event::CONTROLS_STATE:
      handle_controls_state(event.getControlsState(), current_time);
      break;
    case cereal::Event::CAMERA_ODOMETRY:
      handle_camera_odometry(event.getCameraOdometry(), current_time);
      break;
    case cereal::Event::SENSOR_EVENTS:
      handle_sensor_events(event.getSensorEvents(), current_time);
      break;
    default:
      break;
    }

    return type;
  }
};



int main(int argc, char *argv[]) {
  auto ctx = zmq_ctx_new();
  auto controls_state_sock = sub_sock(ctx, "tcp://127.0.0.1:8007");
  auto sensor_events_sock = sub_sock(ctx, "tcp://127.0.0.1:8003");
  auto camera_odometry_sock = sub_sock(ctx, "tcp://127.0.0.1:8066");

  auto live_parameters_sock = zsock_new_pub("@tcp://*:8064");
  assert(live_parameters_sock);
  auto live_parameters_sock_raw = zsock_resolve(live_parameters_sock);

  int err;
  Localizer localizer;

  zmq_pollitem_t polls[num_polls] = {{0}};
  polls[0].socket = controls_state_sock;
  polls[0].events = ZMQ_POLLIN;
  polls[1].socket = sensor_events_sock;
  polls[1].events = ZMQ_POLLIN;
  polls[2].socket = camera_odometry_sock;
  polls[2].events = ZMQ_POLLIN;

  // Read car params
  char *value;
  size_t value_sz = 0;

  LOGW("waiting for params to set vehicle model");
  while (true) {
    read_db_value(NULL, "CarParams", &value, &value_sz);
    if (value_sz > 0) break;
    usleep(100*1000);
  }
  LOGW("got %d bytes CarParams", value_sz);

  // make copy due to alignment issues
  auto amsg = kj::heapArray<capnp::word>((value_sz / sizeof(capnp::word)) + 1);
  memcpy(amsg.begin(), value, value_sz);
  free(value);

  capnp::FlatArrayMessageReader cmsg(amsg);
  cereal::CarParams::Reader car_params = cmsg.getRoot<cereal::CarParams>();

  // Read params from previous run
  const int result = read_db_value(NULL, "LiveParameters", &value, &value_sz);

  std::string fingerprint = car_params.getCarFingerprint();
  std::string vin = car_params.getCarVin();
  double sR = car_params.getSteerRatio();
  double x = 1.0;
  double ao = 0.0;

  if (result == 0){
    auto str = std::string(value, value_sz);
    free(value);

    std::string err;
    auto json = json11::Json::parse(str, err);
    if (json.is_null() || !err.empty()) {
      std::string log = "Error parsing json: " + err;
      LOGW(log.c_str());
    } else {
      std::string new_fingerprint = json["carFingerprint"].string_value();
      std::string new_vin = json["carVin"].string_value();

      if (fingerprint == new_fingerprint && vin == new_vin) {
        std::string log = "Parameter starting with: " + str;
        LOGW(log.c_str());

        sR = json["steerRatio"].number_value();
        x = json["stiffnessFactor"].number_value();
        ao = json["angleOffsetAverage"].number_value();
      }
    }
  }

  ParamsLearner learner(car_params, ao, x, sR, 1.0);

  // Main loop
  int save_counter = 0;
  while (true){
    int ret = zmq_poll(polls, num_polls, 100);

    if (ret == 0){
      continue;
    } else if (ret < 0){
      break;
    }

    for (int i=0; i < num_polls; i++) {
      if (polls[i].revents) {
        zmq_msg_t msg;
        err = zmq_msg_init(&msg);
        assert(err == 0);
        err = zmq_msg_recv(&msg, polls[i].socket, 0);
        assert(err >= 0);
        // make copy due to alignment issues, will be freed on out of scope
        auto amsg = kj::heapArray<capnp::word>((zmq_msg_size(&msg) / sizeof(capnp::word)) + 1);
        memcpy(amsg.begin(), zmq_msg_data(&msg), zmq_msg_size(&msg));

        auto which = localizer.handle_log((const unsigned char*)amsg.begin(), amsg.size());
        zmq_msg_close(&msg);

        if (which == cereal::Event::CONTROLS_STATE){
          save_counter++;

          double yaw_rate = -localizer.x[0];
          bool valid = learner.update(yaw_rate, localizer.car_speed, localizer.steering_angle);

          // TODO: Fix in replay
          double sensor_data_age = localizer.controls_state_time - localizer.sensor_data_time;

          double angle_offset_degrees = RADIANS_TO_DEGREES * learner.ao;
          double angle_offset_average_degrees = RADIANS_TO_DEGREES * learner.slow_ao;

          // Send parameters at 10 Hz
          if (save_counter % 10 == 0){
            capnp::MallocMessageBuilder msg;
            cereal::Event::Builder event = msg.initRoot<cereal::Event>();
            event.setLogMonoTime(nanos_since_boot());
            auto live_params = event.initLiveParameters();
            live_params.setValid(valid);
            live_params.setYawRate(localizer.x[0]);
            live_params.setGyroBias(localizer.x[1]);
            live_params.setSensorValid(sensor_data_age < 5.0);
            live_params.setAngleOffset(angle_offset_degrees);
            live_params.setAngleOffsetAverage(angle_offset_average_degrees);
            live_params.setStiffnessFactor(learner.x);
            live_params.setSteerRatio(learner.sR);

            auto words = capnp::messageToFlatArray(msg);
            auto bytes = words.asBytes();
            zmq_send(live_parameters_sock_raw, bytes.begin(), bytes.size(), ZMQ_DONTWAIT);
          }


          // Save parameters every minute
          if (save_counter % 6000 == 0) {
            json11::Json json = json11::Json::object {
              {"carVin", vin},
              {"carFingerprint", fingerprint},
              {"steerRatio", learner.sR},
              {"stiffnessFactor", learner.x},
              {"angleOffsetAverage", angle_offset_average_degrees},
            };

            std::string out = json.dump();
            write_db_value(NULL, "LiveParameters", out.c_str(), out.length());
          }
        }
      }
    }
  }

  zmq_close(controls_state_sock);
  zmq_close(sensor_events_sock);
  zmq_close(camera_odometry_sock);
  zmq_close(live_parameters_sock_raw);
  return 0;
}


extern "C" {
  void *localizer_init(void) {
    Localizer * localizer = new Localizer;
    return (void*)localizer;
  }

  void localizer_handle_log(void * localizer, const unsigned char * data, size_t len) {
    Localizer * loc = (Localizer*) localizer;
    loc->handle_log(data, len);
  }

  double localizer_get_yaw(void * localizer) {
    Localizer * loc = (Localizer*) localizer;
    return loc->x[0];
  }
  double localizer_get_bias(void * localizer) {
    Localizer * loc = (Localizer*) localizer;
    return loc->x[1];
  }
  double localizer_get_t(void * localizer) {
    Localizer * loc = (Localizer*) localizer;
    return loc->prev_update_time;
  }
}
