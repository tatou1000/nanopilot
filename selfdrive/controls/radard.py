#!/usr/bin/env python3
import importlib
import math
from collections import defaultdict, deque

import zmq

import selfdrive.messaging as messaging
from cereal import car
from common.params import Params
from common.realtime import DT_RDR, Ratekeeper, set_realtime_priority
from selfdrive.config import RADAR_TO_CAMERA
from selfdrive.controls.lib.cluster.fastcluster_py import \
  cluster_points_centroid
from selfdrive.controls.lib.radar_helpers import Cluster, Track
from selfdrive.services import service_list
from selfdrive.swaglog import cloudlog

DEBUG = False

#vision point
DIMSV = 2
XV, SPEEDV = 0, 1
VISION_POINT = -1

# Time-alignment
v_len = 20   # how many speed data points to remember for t alignment with rdr data


def laplacian_cdf(x, mu, b):
  b = max(b, 1e-4)
  return math.exp(-abs(x-mu)/b)


def match_vision_to_cluster(v_ego, lead, clusters):
  # match vision point to best statistical cluster match
  offset_vision_dist = lead.dist - RADAR_TO_CAMERA

  def prob(c):
    prob_d = laplacian_cdf(c.dRel, offset_vision_dist, lead.std)
    prob_y = laplacian_cdf(c.yRel, lead.relY, lead.relYStd)
    prob_v = laplacian_cdf(c.vRel, lead.relVel, lead.relVelStd)

    # This is isn't exactly right, but good heuristic
    return prob_d * prob_y * prob_v

  cluster = max(clusters, key=prob)

  # if no 'sane' match is found return -1
  # stationary radar points can be false positives
  dist_sane = abs(cluster.dRel - offset_vision_dist) < max([(offset_vision_dist)*.25, 5.0])
  vel_sane = (abs(cluster.vRel - lead.relVel) < 10) or (v_ego + cluster.vRel > 2)
  if dist_sane and vel_sane:
    return cluster
  else:
    return None


def get_lead(v_ego, ready, clusters, lead_msg, low_speed_override=True):
  # Determine leads, this is where the essential logic happens
  if len(clusters) > 0 and ready and lead_msg.prob > .5:
    cluster = match_vision_to_cluster(v_ego, lead_msg, clusters)
  else:
    cluster = None

  lead_dict = {'status': False}
  if cluster is not None:
    lead_dict = cluster.get_RadarState(lead_msg.prob)
  elif (cluster is None) and ready and (lead_msg.prob > .5):
    lead_dict = Cluster().get_RadarState_from_vision(lead_msg, v_ego)

  if low_speed_override:
    low_speed_clusters = [c for c in clusters if c.potential_low_speed_lead(v_ego)]
    if len(low_speed_clusters) > 0:
      closest_cluster = min(low_speed_clusters, key=lambda c: c.dRel)

      # Only choose new cluster if it is actually closer than the previous one
      if (not lead_dict['status']) or (closest_cluster.dRel < lead_dict['dRel']):
        lead_dict = closest_cluster.get_RadarState()

  return lead_dict


class RadarD():
  def __init__(self, mocked, delay=0):
    self.current_time = 0
    self.mocked = mocked

    self.tracks = defaultdict(dict)

    self.last_md_ts = 0
    self.last_controls_state_ts = 0

    self.active = 0

    # v_ego
    self.v_ego = 0.
    self.v_ego_hist = deque([0], maxlen=delay+1)

    self.v_ego_t_aligned = 0.
    self.ready = False

  def update(self, frame, sm, rr, has_radar):
    self.current_time = 1e-9*max([sm.logMonoTime[key] for key in sm.logMonoTime.keys()])

    if sm.updated['controlsState']:
      self.active = sm['controlsState'].active
      self.v_ego = sm['controlsState'].vEgo
      self.v_ego_hist.append(self.v_ego)
    if sm.updated['model']:
      self.ready = True

    ar_pts = {}
    for pt in rr.points:
      ar_pts[pt.trackId] = [pt.dRel, pt.yRel, pt.vRel, pt.measured]

    # *** remove missing points from meta data ***
    for ids in list(self.tracks.keys()):
      if ids not in ar_pts:
        self.tracks.pop(ids, None)

    # *** compute the tracks ***
    for ids in ar_pts:
      rpt = ar_pts[ids]

      # align v_ego by a fixed time to align it with the radar measurement
      self.v_ego_t_aligned = self.v_ego_hist[0]

      # create the track if it doesn't exist or it's a new track
      if ids not in self.tracks:
        self.tracks[ids] = Track()
      self.tracks[ids].update(rpt[0], rpt[1], rpt[2], self.v_ego_t_aligned, rpt[3])

    idens = list(sorted(self.tracks.keys()))
    track_pts = list([self.tracks[iden].get_key_for_cluster() for iden in idens])


    # If we have multiple points, cluster them
    if len(track_pts) > 1:
      cluster_idxs = cluster_points_centroid(track_pts, 2.5)
      clusters = [None] * (max(cluster_idxs) + 1)

      for idx in range(len(track_pts)):
        cluster_i = cluster_idxs[idx]
        if clusters[cluster_i] is None:
          clusters[cluster_i] = Cluster()
        clusters[cluster_i].add(self.tracks[idens[idx]])
    elif len(track_pts) == 1:
      # FIXME: cluster_point_centroid hangs forever if len(track_pts) == 1
      cluster_idxs = [0]
      clusters = [Cluster()]
      clusters[0].add(self.tracks[idens[0]])
    else:
      clusters = []

    # if a new point, reset accel to the rest of the cluster
    for idx in range(len(track_pts)):
      if self.tracks[idens[idx]].cnt <= 1:
        aLeadK = clusters[cluster_idxs[idx]].aLeadK
        aLeadTau = clusters[cluster_idxs[idx]].aLeadTau
        self.tracks[idens[idx]].reset_a_lead(aLeadK, aLeadTau)

    # *** publish radarState ***
    dat = messaging.new_message()
    dat.init('radarState')
    dat.valid = sm.all_alive_and_valid(service_list=['controlsState', 'model'])
    dat.radarState.mdMonoTime = self.last_md_ts
    dat.radarState.canMonoTimes = list(rr.canMonoTimes)
    dat.radarState.radarErrors = list(rr.errors)
    dat.radarState.controlsStateMonoTime = self.last_controls_state_ts

    if has_radar:
      dat.radarState.leadOne = get_lead(self.v_ego, self.ready, clusters, sm['model'].lead, low_speed_override=True)
      dat.radarState.leadTwo = get_lead(self.v_ego, self.ready, clusters, sm['model'].leadFuture, low_speed_override=False)
    return dat


# fuses camera and radar data for best lead detection
def radard_thread(sm=None, pm=None, can_sock=None):
  set_realtime_priority(2)

  # wait for stats about the car to come in from controls
  cloudlog.info("radard is waiting for CarParams")
  CP = car.CarParams.from_bytes(Params().get("CarParams", block=True))
  mocked = CP.carName == "mock"
  cloudlog.info("radard got CarParams")

  # import the radar from the fingerprint
  cloudlog.info("radard is importing %s", CP.carName)
  RadarInterface = importlib.import_module('selfdrive.car.%s.radar_interface' % CP.carName).RadarInterface

  can_poller = zmq.Poller()

  if can_sock is None:
    can_sock = messaging.sub_sock(service_list['can'].port)
    can_poller.register(can_sock)

  if sm is None:
    sm = messaging.SubMaster(['model', 'controlsState', 'liveParameters'])

  # *** publish radarState and liveTracks
  if pm is None:
    pm = messaging.PubMaster(['radarState', 'liveTracks'])

  RI = RadarInterface(CP)

  rk = Ratekeeper(1.0 / DT_RDR, print_delay_threshold=None)
  RD = RadarD(mocked, RI.delay)

  has_radar = not CP.radarOffCan

  while 1:
    can_strings = messaging.drain_sock_raw_poller(can_poller, can_sock, wait_for_one=True)
    rr = RI.update(can_strings)

    if rr is None:
      continue

    sm.update(0)

    dat = RD.update(rk.frame, sm, rr, has_radar)
    dat.radarState.cumLagMs = -rk.remaining*1000.

    pm.send('radarState', dat)

    # *** publish tracks for UI debugging (keep last) ***
    tracks = RD.tracks
    dat = messaging.new_message()
    dat.init('liveTracks', len(tracks))

    for cnt, ids in enumerate(sorted(tracks.keys())):
      dat.liveTracks[cnt] = {
        "trackId": ids,
        "dRel": float(tracks[ids].dRel),
        "yRel": float(tracks[ids].yRel),
        "vRel": float(tracks[ids].vRel),
      }
    pm.send('liveTracks', dat)

    rk.monitor_time()


def main(sm=None, pm=None, can_sock=None):
  radard_thread(sm, pm, can_sock)


if __name__ == "__main__":
  main()
