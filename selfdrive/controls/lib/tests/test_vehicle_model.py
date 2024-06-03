import pytest
import math

import numpy as np
from control import StateSpace

from openpilot.selfdrive.car.honda.interface import CarInterface
from openpilot.selfdrive.car.honda.values import CAR
from openpilot.selfdrive.controls.lib.vehicle_model import VehicleModel, dyn_ss_sol, create_dyn_state_matrices


class TestVehicleModel:
  def setup_method(self):
    CP = CarInterface.get_non_essential_params(CAR.HONDA_CIVIC)
    self.VM = VehicleModel(CP)

  def test_round_trip_yaw_rate(self):
    # TODO: fix VM to work at zero speed
    for u in np.linspace(1, 30, num=10):
      for roll in np.linspace(math.radians(-20), math.radians(20), num=11):
        for sa in np.linspace(math.radians(-20), math.radians(20), num=11):
          yr = self.VM.yaw_rate(sa, u, roll)
          new_sa = self.VM.get_steer_from_yaw_rate(yr, u, roll)

          assert sa == pytest.approx(new_sa)

  def test_dyn_ss_sol_against_yaw_rate(self):
    """Verify that the yaw_rate helper function matches the results
    from the state space model."""

    for roll in np.linspace(math.radians(-20), math.radians(20), num=11):
      for u in np.linspace(1, 30, num=10):
        for sa in np.linspace(math.radians(-20), math.radians(20), num=11):

          # Compute yaw rate based on state space model
          _, yr1 = dyn_ss_sol(sa, u, roll, self.VM)

          # Compute yaw rate using direct computations
          yr2 = self.VM.yaw_rate(sa, u, roll)
          assert float(yr1[0]) == pytest.approx(yr2)

  def test_syn_ss_sol_simulate(self):
    """Verifies that dyn_ss_sol matches a simulation"""

    for roll in np.linspace(math.radians(-20), math.radians(20), num=11):
      for u in np.linspace(1, 30, num=10):
        A, B = create_dyn_state_matrices(u, self.VM)

        # Convert to discrete time system
        ss = StateSpace(A, B, np.eye(2), np.zeros((2, 2)))
        ss = ss.sample(0.01)

        for sa in np.linspace(math.radians(-20), math.radians(20), num=11):
          inp = np.array([[sa], [roll]])

          # Simulate for 1 second
          x1 = np.zeros((2, 1))
          for _ in range(100):
            x1 = ss.A @ x1 + ss.B @ inp

          # Compute steady state solution directly
          x2 = dyn_ss_sol(sa, u, roll, self.VM)

          np.testing.assert_almost_equal(x1, x2, decimal=3)
