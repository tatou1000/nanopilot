#!/usr/bin/env python3
import unittest
from datetime import datetime

from laika.helpers import UbloxGnssId

from laika.gps_time import GPSTime
from laika.raw_gnss import GNSSMeasurement
from selfdrive.locationd.laikad import create_measurement_msg


class TestLaikad(unittest.TestCase):

  def test_create_msg_without_errors(self):
    gpstime = GPSTime.from_datetime(datetime.now())
    meas = GNSSMeasurement('G01', gpstime.week, gpstime.tow, {'C1C': 0., 'D1C': 0.}, {'C1C': 0., 'D1C': 0.}, ublox_gnss_id=UbloxGnssId.GPS)
    msg = create_measurement_msg(meas)

    self.assertEqual(msg.constellationId, 'gps')


if __name__ == "__main__":
  unittest.main()
