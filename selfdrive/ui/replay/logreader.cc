#include "selfdrive/ui/replay/logreader.h"

#include <cassert>
#include <bzlib.h>
#include "selfdrive/common/util.h"

static bool decompressBZ2(std::vector<uint8_t> &dest, const char srcData[], size_t srcSize,
                          size_t outputSizeIncrement = 0x100000U) {
  bz_stream strm = {};
  int ret = BZ2_bzDecompressInit(&strm, 0, 0);
  assert(ret == BZ_OK);

  strm.next_in = const_cast<char *>(srcData);
  strm.avail_in = srcSize;
  do {
    strm.next_out = (char *)&dest[strm.total_out_lo32];
    strm.avail_out = dest.size() - strm.total_out_lo32;
    ret = BZ2_bzDecompress(&strm);
    if (ret == BZ_OK && strm.avail_in > 0 && strm.avail_out == 0) {
      dest.resize(dest.size() + outputSizeIncrement);
    }
  } while (ret == BZ_OK);

  BZ2_bzDecompressEnd(&strm);
  dest.resize(strm.total_out_lo32);
  return ret == BZ_STREAM_END;
}

LogReader::~LogReader() {
  for (auto e : events) delete e;
}

bool LogReader::load(const std::string &file) {
  raw_.resize(1024 * 1024 * 64);
  std::string dat = util::read_file(file);
  if (dat.empty() || !decompressBZ2(raw_, dat.data(), dat.size())) {
    LOGW("bz2 decompress failed");
    return false;
  }

  auto insertEidx = [&](CameraType type, const cereal::EncodeIndex::Reader &e) {
    eidx[type][e.getFrameId()] = {e.getSegmentNum(), e.getSegmentId()};
  };

  kj::ArrayPtr<const capnp::word> words((const capnp::word *)raw_.data(), raw_.size() / sizeof(capnp::word));
  while (words.size() > 0) {
    try {
      std::unique_ptr<Event> evt = std::make_unique<Event>(words);
      switch (evt->which) {
        case cereal::Event::ROAD_ENCODE_IDX:
          insertEidx(RoadCam, evt->event.getRoadEncodeIdx());
          break;
        case cereal::Event::DRIVER_ENCODE_IDX:
          insertEidx(DriverCam, evt->event.getDriverEncodeIdx());
          break;
        case cereal::Event::WIDE_ROAD_ENCODE_IDX:
          insertEidx(WideRoadCam, evt->event.getWideRoadEncodeIdx());
          break;
        default:
          break;
      }
      words = kj::arrayPtr(evt->reader.getEnd(), words.end());
      events.push_back(evt.release());
    } catch (const kj::Exception &e) {
      return false;
    }
  }
  std::sort(events.begin(), events.end(), Event::lessThan());
  return true;
}
