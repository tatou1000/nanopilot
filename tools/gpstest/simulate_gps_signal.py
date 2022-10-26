#!/usr/bin/env python3
import os
import random
import argparse
import datetime as dt
import subprocess as sp
from typing import Tuple

from laika.downloader import download_nav
from laika.gps_time import GPSTime
from laika.helpers import ConstellationId

cache_dir = '/tmp/gpstest/'


def download_rinex():
  # TODO: check if there is a better way to get the full brdc file for LimeGPS
  gps_time = GPSTime.from_datetime(dt.datetime.utcnow())
  utc_time = dt.datetime.utcnow() - dt.timedelta(1)
  gps_time = GPSTime.from_datetime(dt.datetime(utc_time.year, utc_time.month, utc_time.day))
  return download_nav(gps_time, cache_dir, ConstellationId.GPS)

def get_coords(lat, lon, s1, s2, o1=0, o2=0) -> Tuple[int, int]:
  lat_add = random.random()*s1 + o1
  lon_add = random.random()*s2 + o2

  lat = ((lat + lat_add + 90) % 180) - 90
  lon = ((lon + lon_add + 180) % 360) - 180
  return round(lat, 5), round(lon, 5)

def get_continuous_coords(lat, lon) -> Tuple[int, int]:
  # continuously move around the world
  return get_coords(lat, lon, 0.01, 0.01)

def get_random_coords(lat, lon) -> Tuple[int, int]:
  # jump around the world
  return get_coords(lat, lon, 20, 20, 10, 20)

def check_availability() -> bool:
  cmd = ["LimeSuite/builddir/LimeUtil/LimeUtil", "--find"]
  output = sp.check_output(cmd)

  if output.strip() == b"":
    return False

  print(f"Device: {output.strip().decode('utf-8')}")
  return True

def main(lat, lon, jump_sim, contin_sim):
  if not os.path.exists('LimeGPS'):
    print("LimeGPS not found run 'setup.sh' first")
    return

  if not os.path.exists('LimeSuite'):
    print("LimeSuite not found run 'setup.sh' first")
    return

  if not check_availability():
    print("No limeSDR device found!")
    return

  rinex_file = download_rinex()

  if lat == 0 and lon == 0:
    lat, lon = get_random_coords(47.2020, 15.7403)

  timeout = None
  if jump_sim:
    timeout = 30

  while True:
    try:
      print(f"starting LimeGPS, Location: {lat},{lon}")
      cmd = ["LimeGPS/LimeGPS", "-e", rinex_file, "-l", f"{lat},{lon},100"]
      sp.check_output(cmd, stderr=sp.PIPE, timeout=timeout)
    except KeyboardInterrupt:
      print("stopping LimeGPS")
      return
    except sp.TimeoutExpired:
      print("LimeGPS timeout reached!")
    except Exception as e:
      out_stderr = e.stderr.decode('utf-8')# pylint:disable=no-member
      if "Device is busy." in out_stderr:
        print("GPS simulation is already running, Device is busy!")
        return

      print(f"LimeGPS crashed: {str(e)}")
      print(f"stderr:\n{e.stderr.decode('utf-8')}")# pylint:disable=no-member

    if contin_sim:
      lat, lon = get_continuous_coords(lat, lon)
    else:
      lat, lon = get_random_coords(lat, lon)

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Simulate static [or random jumping] GPS signal.")
  parser.add_argument("lat", type=float, nargs='?', default=0)
  parser.add_argument("lon", type=float, nargs='?', default=0)
  parser.add_argument("--jump", action="store_true", help="signal that jumps around the world")
  parser.add_argument("--contin", action="store_true", help="continuously/slowly moving around the world")
  args = parser.parse_args()
  main(args.lat, args.lon, args.jump, args.contin)
