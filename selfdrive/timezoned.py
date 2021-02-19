#!/usr/bin/env python3
import json
import os
import time
import subprocess

import requests
from timezonefinder import TimezoneFinder

from common.params import Params
from selfdrive.hardware import TICI
from selfdrive.swaglog import cloudlog


def set_timezone(valid_timezones, timezone):
  if timezone not in valid_timezones:
    cloudlog.error(f"Timezone not supported {timezone}")
    return

  cloudlog.info(f"Setting timezone to {timezone}")
  try:
    if TICI:
      tzpath = os.path.join("/usr/share/zoneinfo/", timezone)
      os.symlink(tzpath, "/data/etc/localtime")
      with open("/data/etc/timezone", "w") as f:
        f.write(timezone)
    else:
      subprocess.check_call(f'sudo timedatectl set-timezone {timezone}', shell=True)
  except subprocess.CalledProcessError:
    cloudlog.exception(f"Error setting timezone to {timezone}")


def main():
  params = Params()
  tf = TimezoneFinder()

  # Get allowed timezones
  valid_timezones = subprocess.check_output('timedatectl list-timezones', shell=True, encoding='utf8').strip().split('\n')

  while True:
    time.sleep(60)

    is_onroad = params.get("IsOffroad") != b"1"
    if is_onroad:
      continue

    # Set based on param
    timezone = params.get("Timezone", encoding='utf8')
    if timezone is not None:
      cloudlog.info("Setting timezone based on param")
      set_timezone(valid_timezones, timezone)
      continue

    location = params.get("LastGPSPosition", encoding='utf8')

    # Find timezone based on IP geolocation if no gps location is available
    if location is None:
      cloudlog.info("Setting timezone based on IP lookup")
      try:
        r = requests.get("https://ipapi.co/timezone", timeout=10)
        if r.status_code == 200:
          set_timezone(valid_timezones, r.text)
        else:
          cloudlog.error(f"Unexpected status code from api {r.status_code}")

        time.sleep(3600)  # Don't make too many API requests
      except requests.exceptions.RequestException:
        cloudlog.exception("Error getting timezone based on IP")
        continue

    # Find timezone by reverse geocoding the last known gps location
    else:
      cloudlog.info("Setting timezone based on GPS location")
      try:
        location = json.loads(location)
      except Exception:
        cloudlog.exception("Error parsing location")
        continue

      timezone = tf.timezone_at(lng=location['longitude'], lat=location['latitude'])
      if timezone is None:
        cloudlog.error(f"No timezone found based on location, {location}")
        continue
      set_timezone(valid_timezones, timezone)


if __name__ == "__main__":
  main()
