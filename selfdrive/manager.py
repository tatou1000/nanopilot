#!/usr/bin/env python
import os
import sys
import time
import importlib
import subprocess
import signal
import traceback
import usb1
from multiprocessing import Process
from common.services import service_list

import zmq

from setproctitle import setproctitle

from selfdrive.swaglog import cloudlog
import selfdrive.messaging as messaging
from selfdrive.thermal import read_thermal
from selfdrive.registration import register

import common.crash

from selfdrive.loggerd.config import ROOT

# comment out anything you don't want to run
managed_processes = {
  "uploader": "selfdrive.loggerd.uploader",
  "controlsd": "selfdrive.controls.controlsd",
  "radard": "selfdrive.controls.radard",
  "calibrationd": "selfdrive.calibrationd.calibrationd",
  "loggerd": "selfdrive.loggerd.loggerd",
  "logmessaged": "selfdrive.logmessaged",
  "logcatd": ("logcatd", ["./logcatd"]),
  "boardd": ("boardd", ["./boardd"]),   # switch to c++ boardd
  "ui": ("ui", ["./ui"]),
  "visiond": ("visiond", ["./visiond"]),
  "sensord": ("sensord", ["./sensord"]), }

running = {}

# due to qualcomm kernel bugs SIGKILLing visiond sometimes causes page table corruption
unkillable_processes = ['visiond']

# processes to end with SIGINT instead of SIGTERM
interrupt_processes = ['loggerd']

car_started_processes = ['controlsd', 'loggerd', 'sensord', 'radard', 'calibrationd', 'visiond']


# ****************** process management functions ******************
def launcher(proc, gctx):
  try:
    # import the process
    mod = importlib.import_module(proc)

    # rename the process
    setproctitle(proc)

    # exec the process
    mod.main(gctx)
  except KeyboardInterrupt:
    cloudlog.info("child %s got ctrl-c" % proc)
  except Exception:
    # can't install the crash handler becuase sys.excepthook doesn't play nice
    # with threads, so catch it here.
    common.crash.capture_exception()
    raise

def nativelauncher(pargs, cwd):
  # exec the process
  os.chdir(cwd)

  # because when extracted from pex zips permissions get lost -_-
  os.chmod(pargs[0], 0o700)

  os.execvp(pargs[0], pargs)

def start_managed_process(name):
  if name in running or name not in managed_processes:
    return
  proc = managed_processes[name]
  if isinstance(proc, basestring):
    cloudlog.info("starting python %s" % proc)
    running[name] = Process(name=name, target=launcher, args=(proc, gctx))
  else:
    pdir, pargs = proc
    cwd = os.path.dirname(os.path.realpath(__file__))
    if pdir is not None:
      cwd = os.path.join(cwd, pdir)
    cloudlog.info("starting process %s" % name)
    running[name] = Process(name=name, target=nativelauncher, args=(pargs, cwd))
  running[name].start()

def kill_managed_process(name):
  if name not in running or name not in managed_processes:
    return
  cloudlog.info("killing %s" % name)

  if name in interrupt_processes:
    os.kill(running[name].pid, signal.SIGINT)
  else:
    running[name].terminate()


  # give it 5 seconds to die
  running[name].join(5.0)
  if running[name].exitcode is None:
    if name in unkillable_processes:
      cloudlog.critical("unkillable process %s failed to exit! rebooting in 15 if it doesn't die" % name)
      running[name].join(15.0)
      if running[name].exitcode is None:
        cloudlog.critical("FORCE REBOOTING PHONE!")
        os.system("date > /sdcard/unkillable_reboot")
        os.system("reboot")
        raise RuntimeError
    else:
      cloudlog.info("killing %s with SIGKILL" % name)
      os.kill(running[name].pid, signal.SIGKILL)
      running[name].join()

  cloudlog.info("%s is dead with %d" % (name, running[name].exitcode))
  del running[name]

def cleanup_all_processes(signal, frame):
  cloudlog.info("caught ctrl-c %s %s" % (signal, frame))
  for name in running.keys():
    kill_managed_process(name)
  sys.exit(0)

# ****************** run loop ******************

def manager_init():
  global gctx

  reg_res = register()
  if reg_res:
    dongle_id, dongle_secret = reg_res
  else:
    raise Exception("server registration failed")

  # set dongle id
  cloudlog.info("dongle id is " + dongle_id)
  os.environ['DONGLE_ID'] = dongle_id
  os.environ['DONGLE_SECRET'] = dongle_secret

  cloudlog.bind_global(dongle_id=dongle_id)
  common.crash.bind_user(dongle_id=dongle_id)

  # set gctx
  gctx = {
    "calibration": {
      "initial_homography": [1.15728010e+00, -4.69379619e-02, 7.46450623e+01,
                             7.99253014e-02, 1.06372458e+00, 5.77762553e+01,
                             9.35543519e-05, -1.65429898e-04, 9.98062699e-01]
    }
  }

def manager_thread():
  # now loop
  context = zmq.Context()
  thermal_sock = messaging.pub_sock(context, service_list['thermal'].port)
  health_sock = messaging.sub_sock(context, service_list['health'].port)

  version = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "common", "version.h")).read().split('"')[1]

  cloudlog.info("manager start %s" % version)
  cloudlog.info(dict(os.environ))

  start_managed_process("logmessaged")
  start_managed_process("logcatd")
  start_managed_process("uploader")
  start_managed_process("ui")

  if os.getenv("NOBOARD") is None:
    # *** wait for the board ***
    wait_for_device()

  # flash the device
  if os.getenv("NOPROG") is None:
    boarddir = os.path.dirname(os.path.abspath(__file__))+"/../board/"
    os.system("cd %s && make" % boarddir)

  start_managed_process("boardd")

  if os.getenv("STARTALL") is not None:
    for p in car_started_processes:
      start_managed_process(p)

  logger_dead = False

  count = 0

  # set 5 second timeout on health socket
  # 5x slower than expected
  health_sock.RCVTIMEO = 5000

  while 1:
    # get health of board, log this in "thermal"
    td = messaging.recv_sock(health_sock, wait=True)
    print td

    # replace thermald
    msg = read_thermal()

    # loggerd is gated based on free space
    statvfs = os.statvfs(ROOT)
    avail = (statvfs.f_bavail * 1.0)/statvfs.f_blocks

    # thermal message now also includes free space
    msg.thermal.freeSpace = avail
    with open("/sys/class/power_supply/battery/capacity") as f:
      msg.thermal.batteryPercent = int(f.read())
    with open("/sys/class/power_supply/battery/status") as f:
      msg.thermal.batteryStatus = f.read().strip()
    thermal_sock.send(msg.to_bytes())
    print msg

    # TODO: add car battery voltage check
    max_temp = max(msg.thermal.cpu0, msg.thermal.cpu1,
                   msg.thermal.cpu2, msg.thermal.cpu3) / 10.0

    # uploader is gated based on the phone temperature
    if max_temp > 85.0:
      cloudlog.info("over temp: %r", max_temp)
      kill_managed_process("uploader")
    elif max_temp < 70.0:
      start_managed_process("uploader")

    if avail < 0.05:
      logger_dead = True

    # start constellation of processes when the car starts
    if not os.getenv("STARTALL"):
      # with 2% left, we killall, otherwise the phone is bricked
      if td is not None and td.health.started and avail > 0.02:
        for p in car_started_processes:
          if p == "loggerd" and logger_dead:
            kill_managed_process(p)
          else:
            start_managed_process(p)
      else:
        logger_dead = False
        for p in car_started_processes:
          kill_managed_process(p)

      # shutdown if the battery gets lower than 10%, we aren't running, and we are discharging
      if msg.thermal.batteryPercent < 5 and msg.thermal.batteryStatus == "Discharging":
        os.system('LD_LIBRARY_PATH="" svc power shutdown')

    # check the status of all processes, did any of them die?
    for p in running:
      cloudlog.debug("   running %s %s" % (p, running[p]))

    # report to server once per minute
    if (count%60) == 0:
      cloudlog.event("STATUS_PACKET",
        running=running.keys(),
        count=count,
        health=(td.to_dict() if td else None),
        thermal=msg.to_dict(),
        version=version)

    count += 1


# optional, build the c++ binaries and preimport the python for speed
def manager_prepare():
  for p in managed_processes:
    proc = managed_processes[p]
    if isinstance(proc, basestring):
      # import this python
      cloudlog.info("preimporting %s" % proc)
      importlib.import_module(proc)
    else:
      # build this process
      cloudlog.info("building %s" % (proc,))
      try:
        subprocess.check_call(["make", "-j4"], cwd=proc[0])
      except subprocess.CalledProcessError:
        # make clean if the build failed
        cloudlog.info("building %s failed, make clean" % (proc, ))
        subprocess.check_call(["make", "clean"], cwd=proc[0])
        subprocess.check_call(["make", "-j4"], cwd=proc[0])

def wait_for_device():
  while 1:
    try:
      context = usb1.USBContext()
      for device in context.getDeviceList(skip_on_error=True):
        if (device.getVendorID() == 0xbbaa and device.getProductID() == 0xddcc) or \
           (device.getVendorID() == 0x0483 and device.getProductID() == 0xdf11):
          handle = device.open()
          handle.claimInterface(0)
          cloudlog.info("found board")
          handle.close()
          return
    except Exception as e:
      print "exception", e,
    print "waiting..."
    time.sleep(1)

def main():
  if os.getenv("NOLOG") is not None:
    del managed_processes['loggerd']
  if os.getenv("NOUPLOAD") is not None:
    del managed_processes['uploader']
  if os.getenv("NOVISION") is not None:
    del managed_processes['visiond']
  if os.getenv("NOBOARD") is not None:
    del managed_processes['boardd']
  if os.getenv("LEAN") is not None:
    del managed_processes['uploader']
    del managed_processes['loggerd']
    del managed_processes['logmessaged']
    del managed_processes['logcatd']
  if os.getenv("NOCONTROL") is not None:
    del managed_processes['controlsd']
    del managed_processes['radard']

  manager_init()
  manager_prepare()
  
  if os.getenv("PREPAREONLY") is not None:
    sys.exit(0)

  try:
    manager_thread()
  except Exception:
    traceback.print_exc()
    common.crash.capture_exception()
  finally:
    cleanup_all_processes(None, None)

if __name__ == "__main__":
  main()
