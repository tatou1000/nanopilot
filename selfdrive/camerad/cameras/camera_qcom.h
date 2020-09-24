#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <czmq.h>
#include <atomic>
#include "messaging.hpp"

#include "msmb_isp.h"
#include "msmb_ispif.h"
#include "msmb_camera.h"
#include "msm_cam_sensor.h"

#include "common/mat.h"
#include "common/visionbuf.h"
#include "common/buffering.h"
#include "common/utilpp.h"

#include "camera_common.h"

#define FRAME_BUF_COUNT 4
#define METADATA_BUF_COUNT 4

#define DEVICE_OP3 0
#define DEVICE_OP3T 1
#define DEVICE_LP3 2

#define NUM_FOCUS 8

#define LP3_AF_DAC_DOWN 366
#define LP3_AF_DAC_UP 634
#define LP3_AF_DAC_M 440
#define LP3_AF_DAC_3SIG 52
#define OP3T_AF_DAC_DOWN 224
#define OP3T_AF_DAC_UP 456
#define OP3T_AF_DAC_M 300
#define OP3T_AF_DAC_3SIG 96

#define FOCUS_RECOVER_PATIENCE 50 // 2.5 seconds of complete blur
#define FOCUS_RECOVER_STEPS 240 // 6 seconds

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CameraState CameraState;

typedef int (*camera_apply_exposure_func)(CameraState *s, int gain, int integ_lines, int frame_length);

typedef struct StreamState {
  struct msm_isp_buf_request buf_request;
  struct msm_vfe_axi_stream_request_cmd stream_req;
  struct msm_isp_qbuf_info qbuf_info[FRAME_BUF_COUNT];
  VisionBuf *bufs;
} StreamState;

typedef struct CameraState {
  int camera_num;
  int camera_id;
  CameraInfo ci;
  int frame_size;

  int device;

  void* ops_sock_handle;
  zsock_t * ops_sock;

  uint32_t pixel_clock;
  uint32_t line_length_pclk;
  unsigned int max_gain;

  unique_fd csid_fd;
  unique_fd csiphy_fd;
  unique_fd sensor_fd;
  unique_fd isp_fd;
  unique_fd eeprom_fd;
  // rear only
  unique_fd ois_fd, actuator_fd;
  uint16_t infinity_dac;

  struct msm_vfe_axi_stream_cfg_cmd stream_cfg;

  size_t eeprom_size;
  uint8_t *eeprom;

  // uint32_t camera_bufs_ids[FRAME_BUF_COUNT];
  FrameMetadata camera_bufs_metadata[FRAME_BUF_COUNT];
  TBuffer camera_tb;

  pthread_mutex_t frame_info_lock;
  FrameMetadata frame_metadata[METADATA_BUF_COUNT];
  int frame_metadata_idx;
  float cur_exposure_frac;
  float cur_gain_frac;
  int cur_gain;
  int cur_frame_length;
  int cur_integ_lines;

  std::atomic<float> digital_gain;

  StreamState ss[3];

  uint64_t last_t;

  camera_apply_exposure_func apply_exposure;

  int16_t focus[NUM_FOCUS];
  uint8_t confidence[NUM_FOCUS];

  float focus_err;

  uint16_t cur_step_pos;
  uint16_t cur_lens_pos;
  uint64_t last_sag_ts;
  float last_sag_acc_z;
  std::atomic<float> lens_true_pos;

  std::atomic<int> self_recover; // af recovery counter, neg is patience, pos is active

  int fps;

  mat3 transform;
} CameraState;


typedef struct MultiCameraState {
  int device;

  unique_fd ispif_fd;
  unique_fd msmcfg_fd;
  unique_fd v4l_fd;

  CameraState rear;
  CameraState front;
} MultiCameraState;

void cameras_init(MultiCameraState *s);
void cameras_open(MultiCameraState *s, VisionBuf *camera_bufs_rear, VisionBuf *camera_bufs_focus, VisionBuf *camera_bufs_stats, VisionBuf *camera_bufs_front);
void cameras_run(MultiCameraState *s);
void cameras_close(MultiCameraState *s);

void camera_autoexposure(CameraState *s, float grey_frac);
void actuator_move(CameraState *s, uint16_t target);
int sensor_write_regs(CameraState *s, struct msm_camera_i2c_reg_array* arr, size_t size, int data_type);

#ifdef __cplusplus
}  // extern "C"
#endif
