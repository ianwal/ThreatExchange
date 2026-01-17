// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#ifndef FFMPEGUTILS_H
#define FFMPEGUTILS_H

#include <vpdq/cpp/hashing/ffmpegwrapper.h>

#include <cstddef>
#include <string>

namespace facebook {
namespace vpdq {
namespace hashing {
namespace ffmpeg {

/** @brief Writes a raw AVFrame to a file (FOR DEBUG ONLY).
 *
 * @param frame The frame to write. Must not be nullptr and its data must not be
 *              nullptr.
 * @param filename Name of output file.
 *
 * @note The output frame can viewed using ffplay directly:
 *
 * ffplay -f rawvideo -pixel_format rgb24 video_size <width>x<height> <filename>
 *
 **/
void saveFrameToFile(AVFramePtr frame, const std::string& filename);

/** @brief Creates an RGB24 AVFrame.
 *
 * @param width Width of the resulting frame.
 * @param height Height of the resulting frame.
 * @return The RGB24 frame.
 **/
AVFramePtr createRGB24Frame(size_t width, size_t height);

} // namespace ffmpeg
} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // FFMPEGUTILS_H
