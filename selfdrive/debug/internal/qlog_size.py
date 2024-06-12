#!/usr/bin/env python3
import argparse
import bz2
from collections import defaultdict

import matplotlib.pyplot as plt

from openpilot.tools.lib.logreader import LogReader

MIN_SIZE = 0.5  # Percent size of total to show as separate entry


def make_pie(msgs, typ):
  msgs_by_type = defaultdict(list)
  for m in msgs:
    msgs_by_type[m.which()].append(m.as_builder().to_bytes())

  length_by_type = {k: len(b"".join(v)) for k, v in msgs_by_type.items()}
  compressed_length_by_type = {k: len(bz2.compress(b"".join(v))) for k, v in msgs_by_type.items()}

  total = sum(compressed_length_by_type.values())
  uncompressed_total = len(b"".join([m.as_builder().to_bytes() for m in msgs]))

  sizes = sorted(compressed_length_by_type.items(), key=lambda kv: kv[1])

  print("name - comp. size (uncomp. size)")
  for (name, sz) in sizes:
    print(f"{name:<22} - {sz / 1024:.2f} kB ({length_by_type[name] / 1024:.2f} kB)")
  print()
  print(f"{typ} - Total {total / 1024:.2f} kB")
  print(f"{typ} - Uncompressed total {uncompressed_total / 1024 / 1024:.2f} MB")

  sizes_large = [(k, sz) for (k, sz) in sizes if sz >= total * MIN_SIZE / 100]
  sizes_large += [('other', sum(sz for (_, sz) in sizes if sz < total * MIN_SIZE / 100))]

  labels, sizes = zip(*sizes_large, strict=True)

  plt.figure()
  plt.title(f"{typ}")
  plt.pie(sizes, labels=labels, autopct='%1.1f%%')


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='View log size breakdown by message type')
  parser.add_argument('route', help='route to use')
  args = parser.parse_args()

  msgs = list(LogReader(args.route))

  make_pie(msgs, 'qlog')
  plt.show()
