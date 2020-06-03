# flake8: noqa

from selfdrive.car import dbc_dict
from cereal import car
Ecu = car.CarParams.Ecu


# Steer torque limits

class SteerLimitParams:
  STEER_MAX = 600                 # max_steer 2048
  STEER_STEP = 1                  # how often we update the steer cmd
  STEER_DELTA_UP = 10             # torque increase per refresh
  STEER_DELTA_DOWN = 20           # torque decrease per refresh
  STEER_DRIVER_ALLOWANCE = 15     # allowed driver torque before start limiting
  STEER_DRIVER_MULTIPLIER = 1     # weight driver torque
  STEER_DRIVER_FACTOR = 1         # from dbc

class CAR:
  CX5 = "Mazda CX-5 2017"

class LKAS_LIMITS:
  STEER_THRESHOLD = 15
  DISABLE_SPEED = 45    # kph
  ENABLE_SPEED = 52     # kph

class Buttons:
  NONE = 0
  SET_PLUS = 1
  SET_MINUS = 2
  RESUME = 3
  CANCEL = 4

FINGERPRINTS = {
  CAR.CX5: [
    # CX-5 2017 GT
    {
      64: 8, 70: 8, 80: 8, 117: 8, 118: 8, 120: 8, 121: 8, 130: 8, 134: 8, 145: 8, 154: 8, 155: 8, 157: 8, 158: 8, 159: 8, 253: 8, 304: 8, 305: 8, 357: 8, 358: 8, 359: 8, 512: 8, 514: 8, 515: 8, 529: 8, 533: 8, 535: 8, 539: 8, 540: 8, 541: 8, 542: 8, 543: 8, 552: 8, 576: 8, 577: 8, 578: 8, 579: 8, 580: 8, 581: 8, 582: 8, 605: 8, 606: 8, 607: 8, 608: 8, 628: 8, 832: 8, 836: 8, 863: 8, 865: 8, 866: 8, 867: 8, 868: 8, 869: 8, 870: 8, 976: 8, 977: 8, 978: 8, 1034: 8, 1045: 8, 1056: 8, 1061: 8, 1067: 8, 1070: 8, 1078: 8, 1080: 8, 1085: 8, 1086: 8, 1088: 8, 1093: 8, 1108: 8, 1114: 8, 1115: 8, 1116: 8, 1139: 8, 1143: 8, 1147: 8, 1154: 8, 1157: 8, 1160: 8, 1163: 8, 1166: 8, 1177: 8, 1178: 8, 1179: 8, 1180: 8, 1183: 8, 1233: 8, 1236: 8, 1237: 8, 1238: 8, 1239: 8, 1241: 8, 1242: 8, 1243: 8, 1244: 8, 1264: 8, 1266: 8, 1267: 8, 1269: 8, 1270: 8, 1271: 8, 1272: 8, 1274: 8, 1275: 8, 1277: 8, 1278: 8, 1409: 8, 1416: 8, 1425: 8, 1430: 8, 1435: 8, 1440: 8, 1446: 8, 1456: 8, 1479: 8
    },

    # CX-5 2019 GTR
    {
      64: 8, 70: 8, 80: 8, 117: 8, 118: 8, 120: 8, 121: 8, 130: 8, 134: 8, 145: 8, 154: 8, 155: 8, 157: 8, 158: 8, 159: 8, 253: 8, 254: 8, 304: 8, 305: 8, 357: 8, 358: 8, 359: 8, 512: 8, 514: 8, 515: 8, 529: 8, 533: 8, 535: 8, 539: 8, 540: 8, 541: 8, 542: 8, 543: 8, 552: 8, 576: 8, 577: 8, 578: 8, 579: 8, 580: 8, 581: 8, 582: 8, 605: 8, 606: 8, 607: 8, 608: 8, 628: 8, 736: 8, 832: 8, 836: 8, 863: 8, 865: 8, 866: 8, 867: 8, 868: 8, 869: 8, 870: 8, 976: 8, 977: 8, 978: 8, 1034: 8, 1045: 8, 1056: 8, 1061: 8, 1067: 8, 1078: 8, 1080: 8, 1085: 8, 1086: 8, 1088: 8, 1093: 8, 1108: 8, 1114: 8, 1115: 8, 1116: 8, 1139: 8, 1143: 8, 1147: 8, 1154: 8, 1157: 8, 1160: 8, 1163: 8, 1166: 8, 1170: 8, 1171: 8, 1173: 8, 1177: 8, 1178: 8, 1179: 8, 1180: 8, 1183: 8, 1233: 8, 1236: 8, 1237: 8, 1238: 8, 1239: 8, 1241: 8, 1242: 8, 1244: 8, 1260: 8, 1264: 8, 1266: 8, 1267: 8, 1269: 8, 1270: 8, 1271: 8, 1272: 8, 1274: 8, 1277: 8, 1278: 8, 1409: 8, 1416: 8, 1425: 8, 1430: 8, 1435: 8, 1440: 8, 1446: 8, 1456: 8, 1479: 8, 1776: 8, 1792: 8, 1872: 8, 1937: 8, 1953: 8, 1968: 8, 2015: 8, 2016: 8, 2024: 8
    },

    # Mazda 6 2017 GT
    {
      64: 8, 70: 8, 80: 8, 117: 8, 118: 8, 120: 8, 121: 8, 130: 8, 134: 8, 145: 8, 154: 8, 155: 8, 157: 8, 158: 8, 159: 8, 253: 8, 304: 8, 305: 8, 357: 8, 358: 8, 359: 8, 512: 8, 514: 8, 515: 8, 529: 8, 533: 8, 535: 8, 539: 8, 540: 8, 541: 8, 542: 8, 543: 8, 552: 8, 576: 8, 577: 8, 578: 8, 579: 8, 580: 8, 581: 8, 582: 8, 605: 8, 606: 8, 607: 8, 628: 8, 832: 8, 836: 8, 863: 8, 865: 8, 866: 8, 867: 8, 868: 8, 869: 8, 870: 8, 976: 8, 977: 8, 978: 8, 1034: 8, 1045: 8, 1056: 8, 1061: 8, 1067: 8, 1070: 8, 1078: 8, 1080: 8, 1085: 8, 1086: 8, 1088: 8, 1093: 8, 1108: 8, 1114: 8, 1115: 8, 1116: 8, 1143: 8, 1147: 8, 1154: 8, 1157: 8, 1160: 8, 1163: 8, 1166: 8, 1177: 8, 1178: 8, 1179: 8, 1180: 8, 1182: 8, 1183: 8, 1233: 8, 1236: 8, 1237: 8, 1238: 8, 1239: 8, 1241: 8, 1242: 8, 1243: 8, 1244: 8, 1264: 8, 1266: 8, 1267: 8, 1269: 8, 1270: 8, 1271: 8, 1272: 8, 1274: 8, 1275: 8, 1277: 8, 1278: 8, 1409: 8, 1416: 8, 1425: 8, 1430: 8, 1435: 8, 1440: 8, 1456: 8, 1479: 8
    },

    # CX-9 2017 Australia
    {
      64: 8, 70: 8, 80: 8, 117: 8, 118: 8, 120: 8, 121: 8, 130: 8, 134: 8, 138: 8, 145: 8, 154: 8, 155: 8, 157: 8, 158: 8, 159: 8, 253: 8, 304: 8, 305: 8, 357: 8, 358: 8, 359: 8, 512: 8, 514: 8, 515: 8, 522: 8, 529: 8, 533: 8, 535: 8, 539: 8, 540: 8, 541: 8, 542: 8, 543: 8, 552: 8, 576: 8, 577: 8, 578: 8, 579: 8, 580: 8, 581: 8, 582: 8, 583: 8, 605: 8, 606: 8, 628: 8, 832: 8, 976: 8, 977: 8, 978: 8, 1034: 8, 1045: 8, 1056: 8, 1061: 8, 1067: 8, 1078: 8, 1085: 8, 1086: 8, 1088: 8, 1093: 8, 1108: 8, 1114: 8, 1115: 8, 1116: 8, 1139: 8, 1143: 8, 1147: 8, 1154: 8, 1157: 8, 1160: 8, 1163: 8, 1166: 8, 1170: 8, 1177: 8, 1180: 8, 1183: 8, 1233: 8, 1236: 8, 1237: 8, 1238: 8, 1239: 8, 1241: 8, 1242: 8, 1243: 8, 1244: 8, 1247: 8, 1264: 8, 1266: 8, 1267: 8, 1269: 8, 1271: 8, 1272: 8, 1274: 8, 1277: 8, 1278: 8, 1409: 8, 1416: 8, 1425: 8, 1430: 8, 1435: 8, 1440: 8, 1446: 8, 1456: 8, 1479: 8
    },
  ],

}

ECU_FINGERPRINT = {
  Ecu.fwdCamera: [579],   # steer torque cmd
}


DBC = {
  CAR.CX5: dbc_dict('mazda_2017', None),
}
