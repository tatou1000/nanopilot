import json
import os
from common.hardware import PC
from common.file_helpers import mkdirs_exists_ok


class MissingAuthConfigError(Exception):
  pass


if PC:
  CONFIG_DIR = os.path.expanduser('~/.comma')
else:
  CONFIG_DIR = "/tmp/.comma"

mkdirs_exists_ok(CONFIG_DIR)


def get_token():
  try:
    with open(os.path.join(CONFIG_DIR, 'auth.json')) as f:
      auth = json.load(f)
      return auth['access_token']
  except Exception:
    raise MissingAuthConfigError('Authenticate with tools/lib/auth.py')


def set_token(token):
  with open(os.path.join(CONFIG_DIR, 'auth.json'), 'w') as f:
    json.dump({'access_token': token}, f)


def clear_token():
  os.unlink(os.path.join(CONFIG_DIR, 'auth.json'))
