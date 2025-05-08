// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

#include <cstdint>
#include <memory>

namespace facebook {
namespace vpdq {
namespace hashing {
namespace ffmpeg {

namespace config {

// Pixel format for the image passed to PDQ
//
// This probably shouldn't be changed unless PDQ expects a different pixel
// format.
constexpr AVPixelFormat get_pixel_format() {
  return AV_PIX_FMT_RGB24;
}

// Downsample method for the image passed to PDQ
//
// Changing this may affect performance and will almost certainly affect the
// output perceptual hash of the frame.
constexpr int get_downsample_method() {
  return SWS_AREA;
}

} // namespace config

// Custom deleters used to wrap FFmpeg objects with smart pointers

/** @brief A custom deleter functor for AVFrame* */
struct AVFrameDeleter {
  void operator()(AVFrame* ptr) const;
};

/** @brief A custom deleter functor for AVPacket* */
struct AVPacketDeleter {
  void operator()(AVPacket* ptr) const;
};

/** @brief A custom deleter functor for SwsContext* */
struct SwsContextDeleter {
  void operator()(SwsContext* ptr) const;
};

/** @brief A custom deleter functor for AVFormatContext* */
struct AVFormatContextDeleter {
  void operator()(AVFormatContext* ptr) const;
};

/** @brief A custom deleter functor for AVCodecContext* */
struct AVCodecContextDeleter {
  void operator()(AVCodecContext* ptr) const;
};

/** @brief A smart pointer wrapper for AVFrame */
using AVFramePtr = std::unique_ptr<AVFrame, AVFrameDeleter>;

/** @brief A smart pointer wrapper for AVPacket */
using AVPacketPtr = std::unique_ptr<AVPacket, AVPacketDeleter>;

/** @brief A smart pointer wrapper for SwsContext */
using SwsContextPtr = std::unique_ptr<SwsContext, SwsContextDeleter>;

/** @brief A smart pointer wrapper for AVFormatContext */
using AVFormatContextPtr =
    std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;

/** @brief A smart pointer wrapper for AVCodecContext */
using AVCodecContextPtr =
    std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;

/**
 * @brief Video wrapper that can open a video file.
 **/
class FFmpegVideo {
 public:
  FFmpegVideo(const std::string& filename);

  /**
   * @brief Create the SwsContext for resizing the video frames.
   *
   * @return True if the swscontext was successfully created otherwise false.
   *
   * @note If the SwsContext fails to be created the swsContext will be nullptr.
   */
  bool createSwsContext();

  // Copy
  FFmpegVideo(const FFmpegVideo&) = delete;
  FFmpegVideo& operator=(const FFmpegVideo&) = delete;

  // Move
  FFmpegVideo(FFmpegVideo&&) = default;
  FFmpegVideo& operator=(FFmpegVideo&&) = default;

  ~FFmpegVideo() = default;

  AVCodecContextPtr codecContext;
  AVFormatContextPtr formatContext;
  SwsContextPtr swsContext;
  unsigned int videoStreamIndex;
  int width;
  int height;
  double frameRate;
};

/**
 * @brief AVFrame implementation of the Frame type class.
 **/
class FFmpegFrame {
 public:
  /** @brief Constructor
   *
   *  @param frame The AVFrame.
   *  @param frameNumber The frame number in the video.
   **/
  FFmpegFrame(AVFramePtr frame, uint64_t frameNumber);

  /** @brief Get the frame number.
   *
   *  @return The frame number.
   **/
  uint64_t get_frame_number() const;

  /** @brief Get the pointer to the frame data buffer to be used for hashing.
   *
   *  @return Pointer to the frame data buffer.
   **/
  unsigned char* get_buffer_ptr();

  // Copy
  FFmpegFrame(const FFmpegFrame&) = delete;
  FFmpegFrame& operator=(const FFmpegFrame&) = delete;

  // Move
  FFmpegFrame(FFmpegFrame&&) = default;
  FFmpegFrame& operator=(FFmpegFrame&&) = default;

  ~FFmpegFrame() = default;

 private:
  AVFramePtr m_frame;
  uint64_t m_frameNumber;
};

} // namespace ffmpeg
} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // FFMPEGWRAPPER_H
