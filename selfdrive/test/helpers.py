import time
from functools import wraps
from nose.tools import nottest

from selfdrive.hardware import PC
from selfdrive.version import training_version, terms_version
from selfdrive.manager.process_config import managed_processes


def set_params_enabled():
  from common.params import Params
  params = Params()
  params.put("HasAcceptedTerms", terms_version)
  params.put("HasCompletedSetup", "1")
  params.put("OpenpilotEnabledToggle", "1")
  params.put("CommunityFeaturesToggle", "1")
  params.put("Passive", "0")
  params.put("CompletedTrainingVersion", training_version)


def phone_only(x):
  if PC:
    return nottest(x)
  else:
    return x


def with_processes(processes, init_time=0):
  def wrapper(func):
    @wraps(func)
    def wrap(*args, **kwargs):
      # start and assert started
      for n, p in enumerate(processes):
        managed_processes[p].start()
        if n < len(processes) - 1:
          time.sleep(init_time)
      assert all(managed_processes[name].proc.exitcode is None for name in processes)

      # call the function
      try:
        func(*args, **kwargs)
        # assert processes are still started
        assert all(managed_processes[name].proc.exitcode is None for name in processes)
      finally:
        for p in processes:
          managed_processes[p].stop()

    return wrap
  return wrapper
