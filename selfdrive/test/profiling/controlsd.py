#!/usr/bin/env python3

import cProfile
import pprofile
import pyprof2calltree

from tools.lib.logreader import LogReader
from selfdrive.controls.controlsd import controlsd_thread
from selfdrive.test.profiling.lib import SubMaster, PubMaster, SubSocket, ReplayDone

BASE_URL = "https://commadataci.blob.core.windows.net/openpilotci/"
SEGMENT = "99c94dc769b5d96e|2019-08-03--14-19-59/2"


if __name__ == "__main__":
  segment = SEGMENT.replace('|', '/')
  rlog_url = f"{BASE_URL}{segment}/rlog.bz2"
  msgs = list(LogReader(rlog_url))

  pm = PubMaster(['sendcan', 'controlsState', 'carState', 'carControl', 'carEvents', 'carParams'])
  sm = SubMaster(msgs, 'can', ['thermal', 'health', 'liveCalibration', 'dMonitoringState', 'plan', 'pathPlan', 'model'])
  can_sock = SubSocket(msgs, 'can')

  # Statistical
  with pprofile.StatisticalProfile()(period=0.00001) as pr:
    try:
      controlsd_thread(sm, pm, can_sock)
    except ReplayDone:
      pass
  pr.dump_stats('cachegrind.out.controlsd_statistical')

  # Deterministic
  pm = PubMaster(['sendcan', 'controlsState', 'carState', 'carControl', 'carEvents', 'carParams'])
  sm = SubMaster(msgs, 'can', ['thermal', 'health', 'liveCalibration', 'dMonitoringState', 'plan', 'pathPlan', 'model'])
  can_sock = SubSocket(msgs, 'can')

  with cProfile.Profile() as pr:
    try:
      controlsd_thread(sm, pm, can_sock)
    except ReplayDone:
      pass
  pyprof2calltree.convert(pr.getstats(), 'cachegrind.out.controlsd_deterministic')
