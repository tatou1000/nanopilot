#!/usr/bin/env python3
from cereal import car
from selfdrive.car import scale_rot_inertia, scale_tire_stiffness, get_safety_config
from selfdrive.car.interfaces import CarInterfaceBase

class CarInterface(CarInterfaceBase):
  @staticmethod
  def get_params(candidate, fingerprint=None, car_fw=None, disable_radar=False):

    ret = CarInterfaceBase.get_std_params(candidate, fingerprint)

    ret.carName = "body"
    ret.safetyConfigs = [get_safety_config(car.CarParams.SafetyModel.body)]

    ret.steerRatio = 0.5
    ret.steerRateCost = 0.5
    ret.steerLimitTimer = 1.0
    ret.steerActuatorDelay = 0.

    ret.mass = 9
    ret.wheelbase = 0.406
    ret.wheelSpeedFactor = 0.008587
    ret.centerToFront = ret.wheelbase * 0.44

    ret.radarOffCan = True
    ret.openpilotLongitudinalControl = True
    ret.steerControlType = car.CarParams.SteerControlType.angle

    ret.rotationalInertia = scale_rot_inertia(ret.mass, ret.wheelbase)

    ret.tireStiffnessFront, ret.tireStiffnessRear = scale_tire_stiffness(ret.mass, ret.wheelbase, ret.centerToFront)

    return ret

  def update(self, c, can_strings):
    self.cp.update_strings(can_strings)

    ret = self.CS.update(self.cp)

    ret.canValid = self.cp.can_valid

    self.CS.out = ret.as_reader()
    return self.CS.out

  def apply(self, c):
    return self.CC.update(c, self.CS)
