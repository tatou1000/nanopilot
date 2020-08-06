#!/usr/bin/env python3

import os
import copy
import json
import numpy as np
import cereal.messaging as messaging
from selfdrive.config import Conversions as CV
from selfdrive.locationd.calibration_helpers import Calibration
from selfdrive.swaglog import cloudlog
from common.params import Params, put_nonblocking
from common.transformations.model import model_height
from common.transformations.camera import view_frame_from_device_frame, get_view_frame_from_road_frame, \
                                          get_calib_from_vp, vp_from_rpy, H, W, FOCAL

MIN_SPEED_FILTER = 15 * CV.MPH_TO_MS
MAX_VEL_ANGLE_STD = np.radians(0.25)
MAX_YAW_RATE_FILTER = np.radians(2)  # per second

# This is all 20Hz, blocks needed for efficiency
BLOCK_SIZE = 100
INPUTS_NEEDED = 5   # allow to update VP every so many frames
INPUTS_WANTED = 50   # We want a little bit more than we need for stability
WRITE_CYCLES = 10  # write every 1000 cycles
VP_INIT = np.array([W/2., H/2.])

# These values are needed to accomodate biggest modelframe
VP_VALIDITY_CORNERS = np.array([[W//2 - 63, 300], [W//2 + 63, 520]])
DEBUG = os.getenv("DEBUG") is not None


def is_calibration_valid(vp):
  return vp[0] > VP_VALIDITY_CORNERS[0, 0] and vp[0] < VP_VALIDITY_CORNERS[1, 0] and \
         vp[1] > VP_VALIDITY_CORNERS[0, 1] and vp[1] < VP_VALIDITY_CORNERS[1, 1]


def sanity_clip(vp):
  if np.isnan(vp).any():
    vp = VP_INIT
  return np.array([np.clip(vp[0], VP_VALIDITY_CORNERS[0, 0] - 5, VP_VALIDITY_CORNERS[1, 0] + 5),
                   np.clip(vp[1], VP_VALIDITY_CORNERS[0, 1] - 5, VP_VALIDITY_CORNERS[1, 1] + 5)])


def intrinsics_from_vp(vp):
  return np.array([
    [FOCAL,   0.,   vp[0]],
    [  0.,  FOCAL,  vp[1]],
    [  0.,    0.,     1.]])


class Calibrator():
  def __init__(self, param_put=False):
    self.param_put = param_put
    self.vp = copy.copy(VP_INIT)
    self.vps = np.zeros((INPUTS_WANTED, 2))
    self.idx = 0
    self.block_idx = 0
    self.valid_blocks = 0
    self.cal_status = Calibration.UNCALIBRATED
    self.just_calibrated = False
    self.v_ego = 0

    # Read calibration
    if param_put:
      calibration_params = Params().get("CalibrationParams")
    else:
      calibration_params = None
    if calibration_params:
      try:
        calibration_params = json.loads(calibration_params)
        self.vp = vp_from_rpy(calibration_params["calib_radians"])
        if not np.isfinite(self.vp).all():
          self.vp = copy.copy(VP_INIT)
        self.vps = np.tile(self.vp, (INPUTS_WANTED, 1))
        self.valid_blocks = calibration_params['valid_blocks']
        if not np.isfinite(self.valid_blocks) or self.valid_blocks < 0:
          self.valid_blocks = 0
        self.update_status()
      except Exception:
        cloudlog.exception("CalibrationParams file found but error encountered")

  def update_status(self):
    start_status = self.cal_status
    if self.valid_blocks < INPUTS_NEEDED:
      self.cal_status = Calibration.UNCALIBRATED
    else:
      self.cal_status = Calibration.CALIBRATED if is_calibration_valid(self.vp) else Calibration.INVALID
    end_status = self.cal_status

    self.just_calibrated = False
    if start_status == Calibration.UNCALIBRATED and end_status != Calibration.UNCALIBRATED:
      self.just_calibrated = True

  def handle_v_ego(self, v_ego):
    self.v_ego = v_ego

  def handle_cam_odom(self, trans, rot, trans_std, rot_std):
    straight_and_fast = ((self.v_ego > MIN_SPEED_FILTER) and (trans[0] > MIN_SPEED_FILTER) and (abs(rot[2]) < MAX_YAW_RATE_FILTER))
    certain_if_calib = ((np.arctan2(trans_std[1], trans[0]) < MAX_VEL_ANGLE_STD) or
                        (self.valid_blocks < INPUTS_NEEDED))
    if straight_and_fast and certain_if_calib:
      # intrinsics are not eon intrinsics, since this is calibrated frame
      intrinsics = intrinsics_from_vp(self.vp)
      new_vp = intrinsics.dot(view_frame_from_device_frame.dot(trans))
      new_vp = new_vp[:2]/new_vp[2]
      new_vp = sanity_clip(new_vp)

      self.vps[self.block_idx] = (self.idx*self.vps[self.block_idx] + (BLOCK_SIZE - self.idx) * new_vp) / float(BLOCK_SIZE)
      self.idx = (self.idx + 1) % BLOCK_SIZE
      if self.idx == 0:
        self.block_idx += 1
        self.valid_blocks = max(self.block_idx, self.valid_blocks)
        self.block_idx = self.block_idx % INPUTS_WANTED
      if self.valid_blocks > 0:
        self.vp = np.mean(self.vps[:self.valid_blocks], axis=0)
      self.update_status()

      if self.param_put and ((self.idx == 0 and self.block_idx == 0) or self.just_calibrated):
        calib = get_calib_from_vp(self.vp)
        cal_params = {"calib_radians": list(calib),
                      "valid_blocks": self.valid_blocks}
        put_nonblocking("CalibrationParams", json.dumps(cal_params).encode('utf8'))
      return new_vp
    else:
      return None

  def send_data(self, pm):
    calib = get_calib_from_vp(self.vp)
    if self.valid_blocks > 0:
      max_vp_calib = np.array(get_calib_from_vp(np.max(self.vps[:self.valid_blocks], axis=0)))
      min_vp_calib = np.array(get_calib_from_vp(np.min(self.vps[:self.valid_blocks], axis=0)))
      calib_spread = np.abs(max_vp_calib - min_vp_calib)
    else:
      calib_spread = np.zeros(3)
    extrinsic_matrix = get_view_frame_from_road_frame(0, calib[1], calib[2], model_height)

    cal_send = messaging.new_message('liveCalibration')
    cal_send.liveCalibration.validBlocks = self.valid_blocks
    cal_send.liveCalibration.calStatus = self.cal_status
    cal_send.liveCalibration.calPerc = min(100 * (self.valid_blocks * BLOCK_SIZE + self.idx) // (INPUTS_NEEDED * BLOCK_SIZE), 100)
    cal_send.liveCalibration.extrinsicMatrix = [float(x) for x in extrinsic_matrix.flatten()]
    cal_send.liveCalibration.rpyCalib = [float(x) for x in calib]
    cal_send.liveCalibration.rpyCalibSpread = [float(x) for x in calib_spread]

    pm.send('liveCalibration', cal_send)


def calibrationd_thread(sm=None, pm=None):
  if sm is None:
    sm = messaging.SubMaster(['cameraOdometry', 'carState'])

  if pm is None:
    pm = messaging.PubMaster(['liveCalibration'])

  calibrator = Calibrator(param_put=True)

  send_counter = 0
  while 1:
    sm.update()

    # if no inputs still publish calibration
    if not sm.updated['carState'] and not sm.updated['cameraOdometry']:
      calibrator.send_data(pm)
      continue

    if sm.updated['carState']:
      calibrator.handle_v_ego(sm['carState'].vEgo)
      if send_counter % 25 == 0:
        calibrator.send_data(pm)
      send_counter += 1

    if sm.updated['cameraOdometry']:
      new_vp = calibrator.handle_cam_odom(sm['cameraOdometry'].trans,
                                          sm['cameraOdometry'].rot,
                                          sm['cameraOdometry'].transStd,
                                          sm['cameraOdometry'].rotStd)

      if DEBUG and new_vp is not None:
        print('got new vp', new_vp)


def main(sm=None, pm=None):
  calibrationd_thread(sm, pm)


if __name__ == "__main__":
  main()
