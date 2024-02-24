// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#ifndef FFMPEGUTILS_H
#define FFMPEGUTILS_H

#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace facebook {
namespace vpdq {
namespace hashing {
namespace ffmpeg {

// Smart pointer wrapper for AVCodecContext
struct AVCodecContextDeleter {
  void operator()(AVCodecContext* ptr) const { avcodec_free_context(&ptr); }
};
using AVCodecContextPtr =
    std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;

// Smart pointer wrapper for AVFrame
struct AVFrameDeleter {
  void operator()(AVFrame* ptr) const {
    if (ptr != nullptr) {
      if (ptr->data[0] != nullptr) {
        // Free memory allocated by image_alloc.
        // See createTargetFrame().
        av_freep(&ptr->data[0]);
      }
      av_frame_free(&ptr);
    }
  }
};
using AVFramePtr = std::unique_ptr<AVFrame, AVFrameDeleter>;

// Smart pointer wrapper for AVPacket
struct AVPacketDeleter {
  void operator()(AVPacket* ptr) const {
    if (ptr != nullptr) {
      av_packet_unref(ptr);
      av_packet_free(&ptr);
    }
  }
};
using AVPacketPtr = std::unique_ptr<AVPacket, AVPacketDeleter>;

// Smart pointer wrapper for AVFormatContext
struct AVFormatContextDeleter {
  void operator()(AVFormatContext* ptr) const { avformat_close_input(&ptr); }
};
using AVFormatContextPtr =
    std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;

// Smart pointer wrapper for SwsContext
struct SwsContextDeleter {
  void operator()(SwsContext* ptr) const { sws_freeContext(ptr); }
};
using SwsContextPtr = std::unique_ptr<SwsContext, SwsContextDeleter>;

} // namespace ffmpeg
} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif
