from cereal import car
from openpilot.selfdrive.car.ford.values import CAR

Ecu = car.CarParams.Ecu

FW_VERSIONS = {
  CAR.BRONCO_SPORT_MK1: {
    (Ecu.eps, 0x730, None): [
      b'LX6C-14D003-AH\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6C-14D003-AK\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.abs, 0x760, None): [
      b'LX6C-2D053-RD\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6C-2D053-RE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdRadar, 0x764, None): [
      b'LB5T-14D049-AB\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdCamera, 0x706, None): [
      b'M1PT-14F397-AC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.engine, 0x7e0, None): [
      b'M1PA-14C204-GF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'M1PA-14C204-RE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'N1PA-14C204-AC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'N1PA-14C204-AD\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
  },
  CAR.ESCAPE_MK4: {
    (Ecu.eps, 0x730, None): [
      b'LX6C-14D003-AF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6C-14D003-AH\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6C-14D003-AL\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.abs, 0x760, None): [
      b'LX6C-2D053-NS\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6C-2D053-NT\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6C-2D053-NY\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6C-2D053-SA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6C-2D053-SD\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdRadar, 0x764, None): [
      b'LB5T-14D049-AB\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdCamera, 0x706, None): [
      b'LJ6T-14F397-AD\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LJ6T-14F397-AE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LV4T-14F397-GG\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.engine, 0x7e0, None): [
      b'LX6A-14C204-BJV\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6A-14C204-BJX\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6A-14C204-CNG\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6A-14C204-DPK\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LX6A-14C204-ESG\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'MX6A-14C204-BEF\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'MX6A-14C204-BEJ\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'MX6A-14C204-CAB\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'NX6A-14C204-BLE\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
  },
  CAR.EXPLORER_MK6: {
    (Ecu.eps, 0x730, None): [
      b'L1MC-14D003-AJ\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'L1MC-14D003-AK\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'L1MC-14D003-AL\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'M1MC-14D003-AB\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'M1MC-14D003-AC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'P1MC-14D003-AA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.abs, 0x760, None): [
      b'L1MC-2D053-AJ\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'L1MC-2D053-BA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'L1MC-2D053-BB\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'L1MC-2D053-BD\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'L1MC-2D053-BF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'L1MC-2D053-KB\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdRadar, 0x764, None): [
      b'LB5T-14D049-AB\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdCamera, 0x706, None): [
      b'LB5T-14F397-AD\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LB5T-14F397-AE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LB5T-14F397-AF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LC5T-14F397-AH\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.engine, 0x7e0, None): [
      b'LB5A-14C204-ATJ\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LB5A-14C204-ATS\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LB5A-14C204-AUJ\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LB5A-14C204-AZL\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LB5A-14C204-BUJ\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LB5A-14C204-EAC\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'MB5A-14C204-LE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'MB5A-14C204-MD\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'MB5A-14C204-RC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'NB5A-14C204-AZD\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'NB5A-14C204-HB\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'PB5A-14C204-DA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
  },
  CAR.F_150_MK14: {
    (Ecu.eps, 0x730, None): [
      b'ML3V-14D003-BC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.abs, 0x760, None): [
      b'PL34-2D053-CA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdRadar, 0x764, None): [
      b'ML3T-14D049-AL\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdCamera, 0x706, None): [
      b'PJ6T-14H102-ABJ\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.engine, 0x7e0, None): [
      b'PL3A-14C204-BRB\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
  },
  CAR.F_150_LIGHTNING_MK1: {
    (Ecu.abs, 0x760, None): [
      b'PL38-2D053-AA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdCamera, 0x706, None): [
      b'ML3T-14H102-ABT\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdRadar, 0x764, None): [
      b'ML3T-14D049-AL\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.engine, 0x7e0, None): [
      b'NL3A-14C204-BAR\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
  },
  CAR.MUSTANG_MACH_E_MK1: {
    (Ecu.eps, 0x730, None): [
      b'LJ9C-14D003-AM\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'LJ9C-14D003-CC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.abs, 0x760, None): [
      b'LK9C-2D053-CK\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdRadar, 0x764, None): [
      b'ML3T-14D049-AL\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdCamera, 0x706, None): [
      b'ML3T-14H102-ABS\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.engine, 0x7e0, None): [
      b'MJ98-14C204-BBS\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'NJ98-14C204-VH\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
  },
  CAR.FOCUS_MK4: {
    (Ecu.eps, 0x730, None): [
      b'JX6C-14D003-AH\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.abs, 0x760, None): [
      b'JX61-2D053-CJ\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdRadar, 0x764, None): [
      b'JX7T-14D049-AC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdCamera, 0x706, None): [
      b'JX7T-14F397-AH\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.engine, 0x7e0, None): [
      b'JX6A-14C204-BPL\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
  },
  CAR.MAVERICK_MK1: {
    (Ecu.eps, 0x730, None): [
      b'NZ6C-14D003-AL\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.abs, 0x760, None): [
      b'NZ6C-2D053-AG\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'PZ6C-2D053-ED\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'PZ6C-2D053-EE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdRadar, 0x764, None): [
      b'NZ6T-14D049-AA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.fwdCamera, 0x706, None): [
      b'NZ6T-14F397-AC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
    (Ecu.engine, 0x7e0, None): [
      b'NZ6A-14C204-AAA\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'NZ6A-14C204-PA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'NZ6A-14C204-ZA\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'NZ6A-14C204-ZC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'PZ6A-14C204-BE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'PZ6A-14C204-JC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
      b'PZ6A-14C204-JE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00',
    ],
  },
}
