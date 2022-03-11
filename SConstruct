import os
import shutil
import subprocess
import sys
import sysconfig
import platform
import numpy as np

TICI = os.path.isfile('/TICI')
Decider('MD5-timestamp')

AddOption('--test',
          action='store_true',
          help='build test files')

AddOption('--extras',
          action='store_true',
          help='build misc extras, like setup and installer files')

AddOption('--kaitai',
          action='store_true',
          help='Regenerate kaitai struct parsers')

AddOption('--asan',
          action='store_true',
          help='turn on ASAN')

AddOption('--ubsan',
          action='store_true',
          help='turn on UBSan')

AddOption('--clazy',
          action='store_true',
          help='build with clazy')

AddOption('--compile_db',
          action='store_true',
          help='build clang compilation database')

AddOption('--snpe',
          action='store_true',
          help='use SNPE on PC')

AddOption('--external-sconscript',
          action='store',
          metavar='FILE',
          dest='external_sconscript',
          help='add an external SConscript to the build')

AddOption('--no-thneed',
          action='store_true',
          dest='no_thneed',
          help='avoid using thneed')

real_arch = arch = subprocess.check_output(["uname", "-m"], encoding='utf8').rstrip()
if platform.system() == "Darwin":
  arch = "Darwin"

if arch == "aarch64" and TICI:
  arch = "larch64"

USE_WEBCAM = os.getenv("USE_WEBCAM") is not None

lenv = {
  "PATH": os.environ['PATH'],
  "LD_LIBRARY_PATH": [Dir(f"#third_party/acados/{arch}/lib").abspath],
  "PYTHONPATH": Dir("#").abspath + ":" + Dir("#pyextra/").abspath,

  "ACADOS_SOURCE_DIR": Dir("#third_party/acados/include/acados").abspath,
  "ACADOS_PYTHON_INTERFACE_PATH": Dir("#pyextra/acados_template").abspath,
  "TERA_PATH": Dir("#").abspath + f"/third_party/acados/{arch}/t_renderer",
}

rpath = lenv["LD_LIBRARY_PATH"].copy()

if arch == "aarch64" or arch == "larch64":
  lenv["LD_LIBRARY_PATH"] += ['/data/data/com.termux/files/usr/lib']

  if arch == "aarch64":
    # android
    lenv["ANDROID_DATA"] = os.environ['ANDROID_DATA']
    lenv["ANDROID_ROOT"] = os.environ['ANDROID_ROOT']

  cpppath = [
    "#third_party/opencl/include",
  ]

  libpath = [
    "/usr/local/lib",
    "/usr/lib",
    "/system/vendor/lib64",
    f"#third_party/acados/{arch}/lib",
  ]

  if arch == "larch64":
    libpath += [
      "#third_party/snpe/larch64",
      "#third_party/libyuv/larch64/lib",
      "/usr/lib/aarch64-linux-gnu"
    ]
    cpppath += [
      "#selfdrive/camerad/include",
    ]
    cflags = ["-DQCOM2", "-mcpu=cortex-a57"]
    cxxflags = ["-DQCOM2", "-mcpu=cortex-a57"]
    rpath += ["/usr/local/lib"]
  else:
    rpath = []
    libpath += [
      "#third_party/snpe/aarch64",
      "#third_party/libyuv/lib",
      "/system/vendor/lib64"
    ]
    cflags = ["-DQCOM", "-D_USING_LIBCXX", "-mcpu=cortex-a57"]
    cxxflags = ["-DQCOM", "-D_USING_LIBCXX", "-mcpu=cortex-a57"]
else:
  cflags = []
  cxxflags = []
  cpppath = []

  # MacOS
  if arch == "Darwin":
    brew_prefix = subprocess.check_output(['brew', '--prefix'], encoding='utf8').strip()
    yuv_dir = "mac" if real_arch != "arm64" else "mac_arm64"
    libpath = [
      f"#third_party/libyuv/{yuv_dir}/lib",
      f"{brew_prefix}/lib",
      f"{brew_prefix}/Library",
      f"{brew_prefix}/opt/openssl/lib",
      f"{brew_prefix}/Cellar",
      f"#third_party/acados/{arch}/lib",
      "/System/Library/Frameworks/OpenGL.framework/Libraries",
    ]
    cflags += ["-DGL_SILENCE_DEPRECATION"]
    cxxflags += ["-DGL_SILENCE_DEPRECATION"]
    cpppath += [
      f"{brew_prefix}/include",
      f"{brew_prefix}/opt/openssl/include",
    ]
  # Linux 86_64
  else:
    libpath = [
      "#third_party/acados/x86_64/lib",
      "#third_party/snpe/x86_64-linux-clang",
      "#third_party/libyuv/x64/lib",
      "#third_party/mapbox-gl-native-qt/x86_64",
      "#cereal",
      "#selfdrive/common",
      "/usr/lib",
      "/usr/local/lib",
    ]

  rpath += [
    Dir("#third_party/snpe/x86_64-linux-clang").abspath,
    Dir("#cereal").abspath,
    Dir("#selfdrive/common").abspath
  ]

if GetOption('asan'):
  ccflags = ["-fsanitize=address", "-fno-omit-frame-pointer"]
  ldflags = ["-fsanitize=address"]
elif GetOption('ubsan'):
  ccflags = ["-fsanitize=undefined"]
  ldflags = ["-fsanitize=undefined"]
else:
  ccflags = []
  ldflags = []

# no --as-needed on mac linker
if arch != "Darwin":
  ldflags += ["-Wl,--as-needed", "-Wl,--no-undefined"]

# Enable swaglog include in submodules
cflags += ['-DSWAGLOG="\\"selfdrive/common/swaglog.h\\""']
cxxflags += ['-DSWAGLOG="\\"selfdrive/common/swaglog.h\\""']

env = Environment(
  ENV=lenv,
  CCFLAGS=[
    "-g",
    "-fPIC",
    "-O2",
    "-Wunused",
    "-Werror",
    "-Wshadow",
    "-Wno-unknown-warning-option",
    "-Wno-deprecated-register",
    "-Wno-register",
    "-Wno-inconsistent-missing-override",
    "-Wno-c99-designator",
    "-Wno-reorder-init-list",
    "-Wno-error=unused-but-set-variable",
  ] + cflags + ccflags,

  CPPPATH=cpppath + [
    "#",
    "#third_party/acados/include",
    "#third_party/acados/include/blasfeo/include",
    "#third_party/acados/include/hpipm/include",
    "#third_party/catch2/include",
    "#third_party/bzip2",
    "#third_party/libyuv/include",
    "#third_party/openmax/include",
    "#third_party/json11",
    "#third_party/curl/include",
    "#third_party/libgralloc/include",
    "#third_party/android_frameworks_native/include",
    "#third_party/android_hardware_libhardware/include",
    "#third_party/android_system_core/include",
    "#third_party/linux/include",
    "#third_party/snpe/include",
    "#third_party/mapbox-gl-native-qt/include",
    "#third_party/qrcode",
    "#third_party",
    "#cereal",
    "#opendbc/can",
  ],

  CC='clang',
  CXX='clang++',
  LINKFLAGS=ldflags,

  RPATH=rpath,

  CFLAGS=["-std=gnu11"] + cflags,
  CXXFLAGS=["-std=c++1z"] + cxxflags,
  LIBPATH=libpath + [
    "#cereal",
    "#third_party",
    "#opendbc/can",
    "#selfdrive/boardd",
    "#selfdrive/common",
  ],
  CYTHONCFILESUFFIX=".cpp",
  COMPILATIONDB_USE_ABSPATH=True,
  tools=["default", "cython", "compilation_db"],
)

if arch == "Darwin":
  env['RPATHPREFIX'] = "-rpath "

if GetOption('compile_db'):
  env.CompilationDatabase('compile_commands.json')

# Setup cache dir
cache_dir = '/data/scons_cache' if TICI else '/tmp/scons_cache'
CacheDir(cache_dir)
Clean(["."], cache_dir)

node_interval = 5
node_count = 0
def progress_function(node):
  global node_count
  node_count += node_interval
  sys.stderr.write("progress: %d\n" % node_count)

if os.environ.get('SCONS_PROGRESS'):
  Progress(progress_function, interval=node_interval)

SHARED = False

def abspath(x):
  if arch == 'aarch64':
    pth = os.path.join("/data/pythonpath", x[0].path)
    env.Depends(pth, x)
    return File(pth)
  else:
    # rpath works elsewhere
    return x[0].path.rsplit("/", 1)[1][:-3]

# Cython build enviroment
py_include = sysconfig.get_paths()['include']
envCython = env.Clone()
envCython["CPPPATH"] += [py_include, np.get_include()]
envCython["CCFLAGS"] += ["-Wno-#warnings", "-Wno-shadow", "-Wno-deprecated-declarations"]

envCython["LIBS"] = []
if arch == "Darwin":
  envCython["LINKFLAGS"] = ["-bundle", "-undefined", "dynamic_lookup"]
elif arch == "aarch64":
  envCython["LINKFLAGS"] = ["-shared"]
  envCython["LIBS"] = [os.path.basename(py_include)]
else:
  envCython["LINKFLAGS"] = ["-pthread", "-shared"]

Export('envCython')

# Qt build environment
qt_env = env.Clone()
qt_modules = ["Widgets", "Gui", "Core", "Network", "Concurrent", "Multimedia", "Quick", "Qml", "QuickWidgets", "Location", "Positioning"]
if arch != "aarch64":
  qt_modules += ["DBus"]

qt_libs = []
if arch == "Darwin":
  if real_arch == "arm64":
    qt_env['QTDIR'] = "/opt/homebrew/opt/qt@5"
  else:
    qt_env['QTDIR'] = "/usr/local/opt/qt@5"
  qt_dirs = [
    os.path.join(qt_env['QTDIR'], "include"),
  ]
  qt_dirs += [f"{qt_env['QTDIR']}/include/Qt{m}" for m in qt_modules]
  qt_env["LINKFLAGS"] += ["-F" + os.path.join(qt_env['QTDIR'], "lib")]
  qt_env["FRAMEWORKS"] += [f"Qt{m}" for m in qt_modules] + ["OpenGL"]
  qt_env.AppendENVPath('PATH', os.path.join(qt_env['QTDIR'], "bin"))
elif arch == "aarch64":
  qt_env['QTDIR'] = "/usr"
  qt_dirs = [
    f"/usr/include/qt",
  ]
  qt_dirs += [f"/usr/include/qt/Qt{m}" for m in qt_modules]

  qt_libs = [f"Qt5{m}" for m in qt_modules]
  qt_libs += ['EGL', 'GLESv3', 'c++_shared']
else:
  qt_env['QTDIR'] = "/usr"
  qt_dirs = [
    f"/usr/include/{real_arch}-linux-gnu/qt5",
    f"/usr/include/{real_arch}-linux-gnu/qt5/QtGui/5.12.8/QtGui",
  ]
  qt_dirs += [f"/usr/include/{real_arch}-linux-gnu/qt5/Qt{m}" for m in qt_modules]

  qt_libs = [f"Qt5{m}" for m in qt_modules]
  if arch == "larch64":
    qt_libs += ["GLESv2", "wayland-client"]
  elif arch != "Darwin":
    qt_libs += ["GL"]

qt_env.Tool('qt')
qt_env['CPPPATH'] += qt_dirs + ["#selfdrive/ui/qt/"]
qt_flags = [
  "-D_REENTRANT",
  "-DQT_NO_DEBUG",
  "-DQT_WIDGETS_LIB",
  "-DQT_GUI_LIB",
  "-DQT_QUICK_LIB",
  "-DQT_QUICKWIDGETS_LIB",
  "-DQT_QML_LIB",
  "-DQT_CORE_LIB",
  "-DQT_MESSAGELOGCONTEXT",
]
qt_env['CXXFLAGS'] += qt_flags
qt_env['LIBPATH'] += ['#selfdrive/ui']
qt_env['LIBS'] = qt_libs

if GetOption("clazy"):
  checks = [
    "level0",
    "level1",
    "no-range-loop",
    "no-non-pod-global-static",
  ]
  qt_env['CXX'] = 'clazy'
  qt_env['ENV']['CLAZY_IGNORE_DIRS'] = qt_dirs[0]
  qt_env['ENV']['CLAZY_CHECKS'] = ','.join(checks)

Export('env', 'qt_env', 'arch', 'real_arch', 'SHARED', 'USE_WEBCAM')

SConscript(['selfdrive/common/SConscript'])
Import('_common', '_gpucommon', '_gpu_libs')

if SHARED:
  common, gpucommon = abspath(common), abspath(gpucommon)
else:
  common = [_common, 'json11']
  gpucommon = [_gpucommon] + _gpu_libs

Export('common', 'gpucommon')

# cereal and messaging are shared with the system
SConscript(['cereal/SConscript'])
if SHARED:
  cereal = abspath([File('cereal/libcereal_shared.so')])
  messaging = abspath([File('cereal/libmessaging_shared.so')])
else:
  cereal = [File('#cereal/libcereal.a')]
  messaging = [File('#cereal/libmessaging.a')]
  visionipc = [File('#cereal/libvisionipc.a')]

Export('cereal', 'messaging', 'visionipc')

# Build rednose library and ekf models

rednose_config = {
  'generated_folder': '#selfdrive/locationd/models/generated',
  'to_build': {
    'live': ('#selfdrive/locationd/models/live_kf.py', True, ['live_kf_constants.h']),
    'car': ('#selfdrive/locationd/models/car_kf.py', True, []),
  },
}

if arch not in ["aarch64", "larch64"]:
  rednose_config['to_build'].update({
    'gnss': ('#selfdrive/locationd/models/gnss_kf.py', True, []),
    'loc_4': ('#selfdrive/locationd/models/loc_kf.py', True, []),
    'pos_computer_4': ('#rednose/helpers/lst_sq_computer.py', False, []),
    'pos_computer_5': ('#rednose/helpers/lst_sq_computer.py', False, []),
    'feature_handler_5': ('#rednose/helpers/feature_handler.py', False, []),
    'lane': ('#xx/pipeline/lib/ekf/lane_kf.py', True, []),
  })

Export('rednose_config')
SConscript(['rednose/SConscript'])

# Build openpilot

SConscript(['cereal/SConscript'])
SConscript(['panda/board/SConscript'])
SConscript(['opendbc/can/SConscript'])

SConscript(['third_party/SConscript'])

SConscript(['common/SConscript'])
SConscript(['common/kalman/SConscript'])
SConscript(['common/transformations/SConscript'])

SConscript(['selfdrive/camerad/SConscript'])
SConscript(['selfdrive/modeld/SConscript'])

SConscript(['selfdrive/controls/lib/cluster/SConscript'])
SConscript(['selfdrive/controls/lib/lateral_mpc_lib/SConscript'])
SConscript(['selfdrive/controls/lib/longitudinal_mpc_lib/SConscript'])

SConscript(['selfdrive/boardd/SConscript'])
SConscript(['selfdrive/proclogd/SConscript'])
SConscript(['selfdrive/clocksd/SConscript'])

SConscript(['selfdrive/loggerd/SConscript'])

SConscript(['selfdrive/locationd/SConscript'])
SConscript(['selfdrive/sensord/SConscript'])
SConscript(['selfdrive/ui/SConscript'])

if arch != "Darwin":
  SConscript(['selfdrive/logcatd/SConscript'])

if GetOption('test'):
  SConscript('panda/tests/safety/SConscript')

external_sconscript = GetOption('external_sconscript')
if external_sconscript:
  SConscript([external_sconscript])
