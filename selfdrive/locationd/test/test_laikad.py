#!/usr/bin/env python3
import time
import unittest
from datetime import datetime
from unittest import mock
from unittest.mock import Mock, patch

from common.params import Params
from laika.ephemeris import EphemerisType
from laika.gps_time import GPSTime
from laika.helpers import ConstellationId, TimeRangeHolder
from laika.raw_gnss import GNSSMeasurement, read_raw_ublox
from selfdrive.locationd.laikad import EPHEMERIS_CACHE, Laikad, create_measurement_msg
from selfdrive.test.openpilotci import get_url
from tools.lib.logreader import LogReader


def get_log(segs=range(0)):
  logs = []
  for i in segs:
    logs.extend(LogReader(get_url("4cf7a6ad03080c90|2021-09-29--13-46-36", i)))
  return [m for m in logs if m.which() == 'ubloxGnss']


def verify_messages(lr, laikad, return_one_success=False):
  good_msgs = []
  for m in lr:
    msg = laikad.process_ublox_msg(m.ubloxGnss, m.logMonoTime, block=True)
    if msg is not None and len(msg.gnssMeasurements.correctedMeasurements) > 0:
      good_msgs.append(msg)
      if return_one_success:
        return msg
  return good_msgs


class TestLaikad(unittest.TestCase):

  @classmethod
  def setUpClass(cls):
    cls.logs = get_log(range(1))

  def setUp(self):
    Params().delete(EPHEMERIS_CACHE)

  def test_create_msg_without_errors(self):
    gpstime = GPSTime.from_datetime(datetime.now())
    meas = GNSSMeasurement(ConstellationId.GPS, 1, gpstime.week, gpstime.tow, {'C1C': 0., 'D1C': 0.}, {'C1C': 0., 'D1C': 0.})
    # Fake observables_final to be correct
    meas.observables_final = meas.observables
    msg = create_measurement_msg(meas)

    self.assertEqual(msg.constellationId, 'gps')

  def test_laika_online(self):
    laikad = Laikad(auto_update=True, valid_ephem_types=EphemerisType.ULTRA_RAPID_ORBIT)
    correct_msgs = verify_messages(self.logs, laikad)

    correct_msgs_expected = 560
    self.assertEqual(correct_msgs_expected, len(correct_msgs))
    self.assertEqual(correct_msgs_expected, len([m for m in correct_msgs if m.gnssMeasurements.positionECEF.valid]))

  def test_laika_online_nav_only(self):
    laikad = Laikad(auto_update=True, valid_ephem_types=EphemerisType.NAV)
    # Disable fetch_orbits to test NAV only
    laikad.fetch_orbits = Mock()
    correct_msgs = verify_messages(self.logs, laikad)
    correct_msgs_expected = 560
    self.assertEqual(correct_msgs_expected, len(correct_msgs))
    self.assertEqual(correct_msgs_expected, len([m for m in correct_msgs if m.gnssMeasurements.positionECEF.valid]))

  @mock.patch('laika.downloader.download_and_cache_file')
  def test_laika_offline(self, downloader_mock):
    downloader_mock.side_effect = IOError
    laikad = Laikad(auto_update=False)
    correct_msgs = verify_messages(self.logs, laikad)
    self.assertEqual(256, len(correct_msgs))
    self.assertEqual(256, len([m for m in correct_msgs if m.gnssMeasurements.positionECEF.valid]))

  def get_first_gps_time(self):
    for m in self.logs:
      if m.ubloxGnss.which == 'measurementReport':
        new_meas = read_raw_ublox(m.ubloxGnss.measurementReport)
        if len(new_meas) != 0:
          return new_meas[0].recv_time

  def test_laika_get_orbits(self):
    laikad = Laikad(auto_update=False)
    first_gps_time = self.get_first_gps_time()
    # Pretend process has loaded the orbits on startup by using the time of the first gps message.
    laikad.fetch_orbits(first_gps_time, block=True)
    self.dict_has_values(laikad.astro_dog.orbits)

  @unittest.skip("Use to debug live data")
  def test_laika_get_orbits_now(self):
    laikad = Laikad(auto_update=False)
    laikad.fetch_orbits(GPSTime.from_datetime(datetime.utcnow()), block=True)
    prn = "G01"
    self.assertGreater(len(laikad.astro_dog.orbits[prn]), 0)
    prn = "R01"
    self.assertGreater(len(laikad.astro_dog.orbits[prn]), 0)
    print(min(laikad.astro_dog.orbits[prn], key=lambda e: e.epoch).epoch.as_datetime())

  def test_get_orbits_in_process(self):
    laikad = Laikad(auto_update=False)
    has_orbits = False
    for m in self.logs:
      laikad.process_ublox_msg(m.ubloxGnss, m.logMonoTime, block=False)
      if laikad.orbit_fetch_future is not None:
        laikad.orbit_fetch_future.result()
      vals = laikad.astro_dog.orbits.values()
      has_orbits = len(vals) > 0 and max([len(v) for v in vals]) > 0
      if has_orbits:
        break
    self.assertTrue(has_orbits)
    self.assertGreater(len(laikad.astro_dog.orbit_fetched_times._ranges), 0)
    self.assertEqual(None, laikad.orbit_fetch_future)

  def test_cache(self):
    laikad = Laikad(auto_update=True, save_ephemeris=True)
    first_gps_time = self.get_first_gps_time()

    def wait_for_cache():
      max_time = 2
      while Params().get(EPHEMERIS_CACHE) is None:
        time.sleep(0.1)
        max_time -= 0.1
        if max_time == 0:
          self.fail("Cache has not been written after 2 seconds")
    # Test cache with no ephemeris
    laikad.cache_ephemeris(t=GPSTime(0, 0))
    wait_for_cache()
    Params().delete(EPHEMERIS_CACHE)

    laikad.astro_dog.get_navs(first_gps_time)
    laikad.fetch_orbits(first_gps_time, block=True)

    # Wait for cache to save
    wait_for_cache()

    # Check both nav and orbits separate
    laikad = Laikad(auto_update=False, valid_ephem_types=EphemerisType.NAV)
    # Verify orbits and nav are loaded from cache
    self.dict_has_values(laikad.astro_dog.orbits)
    self.dict_has_values(laikad.astro_dog.nav)
    # Verify cache is working for only nav by running a segment
    msg = verify_messages(self.logs, laikad, return_one_success=True)
    self.assertIsNotNone(msg)

    with patch('selfdrive.locationd.laikad.get_orbit_data', return_value=None) as mock_method:
      # Verify no orbit downloads even if orbit fetch times is reset since the cache has recently been saved and we don't want to download high frequently
      laikad.astro_dog.orbit_fetched_times = TimeRangeHolder()
      laikad.fetch_orbits(first_gps_time, block=False)
      mock_method.assert_not_called()

      # Verify cache is working for only orbits by running a segment
      laikad = Laikad(auto_update=False, valid_ephem_types=EphemerisType.ULTRA_RAPID_ORBIT)
      msg = verify_messages(self.logs, laikad, return_one_success=True)
      self.assertIsNotNone(msg)
      # Verify orbit data is not downloaded
      mock_method.assert_not_called()

  def dict_has_values(self, dct):
    self.assertGreater(len(dct), 0)
    self.assertGreater(min([len(v) for v in dct.values()]), 0)


if __name__ == "__main__":
  unittest.main()
