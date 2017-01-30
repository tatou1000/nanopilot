#!/usr/bin/env python
import os
import zmq
import numpy as np
import numpy.matlib
from collections import defaultdict

from fastcluster import linkage_vector

import selfdrive.messaging as messaging
from selfdrive.controls.lib.latcontrol import calc_lookahead_offset
from selfdrive.controls.lib.pathplanner import PathPlanner
from selfdrive.config import VehicleParams
from selfdrive.controls.lib.radar_helpers import Track, Cluster, fcluster, RDR_TO_LDR

from common.services import service_list
from common.realtime import sec_since_boot, set_realtime_priority, Ratekeeper
from common.kalman.ekf import EKF, SimpleSensor

radar_type = os.getenv("RADAR")
if radar_type is not None:
  exec('from selfdrive.radar.'+car_type+'.interface import RadarInterface')
else:
  from selfdrive.radar.nidec.interface import RadarInterface

#vision point
DIMSV = 2
XV, SPEEDV = 0, 1
VISION_POINT = -1

class EKFV1D(EKF):
  def __init__(self):
    super(EKFV1D, self).__init__(False)
    self.identity = np.matlib.identity(DIMSV)
    self.state = np.matlib.zeros((DIMSV, 1))
    self.var_init = 1e2   # ~ model variance when probability is 70%, so good starting point
    self.covar = self.identity * self.var_init

    self.process_noise = np.matlib.diag([0.5, 1])

  def calc_transfer_fun(self, dt):
    tf = np.matlib.identity(DIMSV)
    tf[XV, SPEEDV] = dt
    tfj = tf
    return tf, tfj


# fuses camera and radar data for best lead detection
def radard_thread(gctx=None):
  set_realtime_priority(1)

  context = zmq.Context()

  # *** subscribe to features and model from visiond
  model = messaging.sub_sock(context, service_list['model'].port)
  live100 = messaging.sub_sock(context, service_list['live100'].port)

  PP = PathPlanner(model)
  RI = RadarInterface()

  # *** publish live20 and liveTracks
  live20 = messaging.pub_sock(context, service_list['live20'].port)
  liveTracks = messaging.pub_sock(context, service_list['liveTracks'].port)

  # subscribe to stats about the car
  # TODO: move this to new style packet
  VP = VehicleParams(False, False)  # same for ILX and civic

  path_x = np.arange(0.0, 140.0, 0.1)    # 140 meters is max

  # Time-alignment
  rate = 20.   # model and radar are both at 20Hz
  tsv = 1./rate
  rdr_delay = 0.10   # radar data delay in s
  v_len = 20         # how many speed data points to remember for t alignment with rdr data

  enabled = 0
  steer_angle = 0.

  tracks = defaultdict(dict)

  # Kalman filter stuff: 
  ekfv = EKFV1D()
  speedSensorV = SimpleSensor(XV, 1, 2)

  # v_ego
  v_ego = None
  v_ego_array = np.zeros([2, v_len])
  v_ego_t_aligned = 0.

  rk = Ratekeeper(rate, print_delay_threshold=np.inf)
  while 1:
    rr = RI.update()

    ar_pts = {}
    for pt in rr.points:
      ar_pts[pt.trackId] = [pt.dRel + RDR_TO_LDR, pt.yRel, pt.vRel, pt.aRel, None, False, None]

    # receive the live100s
    l100 = messaging.recv_sock(live100)
    if l100 is not None:
      enabled = l100.live100.enabled
      v_ego = l100.live100.vEgo
      steer_angle = l100.live100.angleSteers

      v_ego_array = np.append(v_ego_array, [[v_ego], [float(rk.frame)/rate]], 1)
      v_ego_array = v_ego_array[:, 1:]

    if v_ego is None:
      continue

    # *** get path prediction from the model ***
    PP.update(sec_since_boot(), v_ego)

    # run kalman filter only if prob is high enough
    if PP.lead_prob > 0.7:
      ekfv.update(speedSensorV.read(PP.lead_dist, covar=PP.lead_var))
      ekfv.predict(tsv)
      ar_pts[VISION_POINT] = (float(ekfv.state[XV]), np.polyval(PP.d_poly, float(ekfv.state[XV])),
                              float(ekfv.state[SPEEDV]), np.nan, PP.logMonoTime, np.nan, sec_since_boot())
    else:
      ekfv.state[XV] = PP.lead_dist
      ekfv.covar = (np.diag([PP.lead_var, ekfv.var_init]))
      ekfv.state[SPEEDV] = 0.
      if VISION_POINT in ar_pts:
        del ar_pts[VISION_POINT]

    # *** compute the likely path_y ***
    if enabled:    # use path from model path_poly
      path_y = np.polyval(PP.d_poly, path_x)
    else:          # use path from steer, set angle_offset to 0 since calibration does not exactly report the physical offset
      path_y = calc_lookahead_offset(v_ego, steer_angle, path_x, VP, angle_offset=0)[0]

    # *** remove missing points from meta data ***
    for ids in tracks.keys():
      if ids not in ar_pts:
        tracks.pop(ids, None)

    # *** compute the tracks ***
    for ids in ar_pts:
      # ignore the vision point for now
      if ids == VISION_POINT:
        continue
      rpt = ar_pts[ids]

      # align v_ego by a fixed time to align it with the radar measurement     
      cur_time = float(rk.frame)/rate
      v_ego_t_aligned = np.interp(cur_time - rdr_delay, v_ego_array[1], v_ego_array[0])
      d_path = np.sqrt(np.amin((path_x - rpt[0]) ** 2 + (path_y - rpt[1]) ** 2))

      # create the track if it doesn't exist or it's a new track
      if ids not in tracks or rpt[5] == 1:
        tracks[ids] = Track()
      tracks[ids].update(rpt[0], rpt[1], rpt[2], d_path, v_ego_t_aligned)

      # allow the vision model to remove the stationary flag if distance and rel speed roughly match
      if VISION_POINT in ar_pts:
        dist_to_vision = np.sqrt((0.5*(ar_pts[VISION_POINT][0] - rpt[0])) ** 2 + (2*(ar_pts[VISION_POINT][1] - rpt[1])) ** 2)
        rel_speed_diff = abs(ar_pts[VISION_POINT][2] - rpt[2])
        tracks[ids].mix_vision(dist_to_vision, rel_speed_diff)

    # publish tracks (debugging)
    dat = messaging.new_message()
    dat.init('liveTracks', len(tracks))
    for cnt, ids in enumerate(tracks.keys()):
      dat.liveTracks[cnt].trackId = ids
      dat.liveTracks[cnt].dRel = float(tracks[ids].dRel)
      dat.liveTracks[cnt].yRel = float(tracks[ids].yRel)
      dat.liveTracks[cnt].vRel = float(tracks[ids].vRel)
      dat.liveTracks[cnt].aRel = float(tracks[ids].aRel)
      dat.liveTracks[cnt].stationary = tracks[ids].stationary
      dat.liveTracks[cnt].oncoming = tracks[ids].oncoming
    liveTracks.send(dat.to_bytes())

    idens = tracks.keys()
    track_pts = np.array([tracks[iden].get_key_for_cluster() for iden in idens])

    # If we have multiple points, cluster them
    if len(track_pts) > 1:
      link = linkage_vector(track_pts, method='centroid')
      cluster_idxs = fcluster(link, 2.5, criterion='distance')
      clusters = [None]*max(cluster_idxs)

      for idx in xrange(len(track_pts)):
        cluster_i = cluster_idxs[idx]-1

        if clusters[cluster_i] == None:
          clusters[cluster_i] = Cluster()
        clusters[cluster_i].add(tracks[idens[idx]])
    elif len(track_pts) == 1:
      # TODO: why do we need this?
      clusters = [Cluster()]
      clusters[0].add(tracks[idens[0]])
    else:
      clusters = []

    # *** extract the lead car ***
    lead_clusters = [c for c in clusters
                     if c.is_potential_lead(v_ego)]
    lead_clusters.sort(key=lambda x: x.dRel)
    lead_len = len(lead_clusters)

    # *** extract the second lead from the whole set of leads ***
    lead2_clusters = [c for c in lead_clusters
                      if c.is_potential_lead2(lead_clusters)]
    lead2_clusters.sort(key=lambda x: x.dRel)
    lead2_len = len(lead2_clusters)

    # *** publish live20 ***
    dat = messaging.new_message()
    dat.init('live20')
    dat.live20.mdMonoTime = PP.logMonoTime
    dat.live20.canMonoTimes = list(rr.canMonoTimes)
    if lead_len > 0:
      lead_clusters[0].toLive20(dat.live20.leadOne)
      if lead2_len > 0:
        lead2_clusters[0].toLive20(dat.live20.leadTwo)
      else:
        dat.live20.leadTwo.status = False
    else:
      dat.live20.leadOne.status = False

    dat.live20.cumLagMs = -rk.remaining*1000.
    live20.send(dat.to_bytes())

    rk.monitor_time()

def main(gctx=None):
  radard_thread(gctx)

if __name__ == "__main__":
  main()
