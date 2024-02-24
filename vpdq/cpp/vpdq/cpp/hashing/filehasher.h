// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#ifndef FILEHASHER_H
#define FILEHASHER_H

#include <string>
#include <vector>

#include <pdq/cpp/common/pdqhashtypes.h>
#include <vpdq/cpp/hashing/vpdqHashType.h>

namespace facebook {
namespace vpdq {
namespace hashing {

/**
 * Get frames from the video
 * Then get pdq hashes for selected frames every secondPerHash
 *
 * @param inputVideoFileName Input video file name.
 * @param pdqHashes          Vector for the output hashes.
 *
 * @return Video hashed successfully.
 *
 * @note If hashing fails for any reason the result will be empty.
 */
bool hashVideoFile(
    const std::string& inputVideoFileName,
    std::vector<hashing::vpdqFeature>& pdqHashes);

/**
 * Get frames from the video
 * Then get pdq hashes for selected frames every secondPerHash
 *
 * @param inputVideoFileName Input video's name
 * @param pdqHashes Vector which stores hashes
 * @param verbose If produce detailed output for diagnostic purposes
 * @param secondsPerHash The time period of picking frames in vpdq
 * @param downsampleWidth Width to downsample to before hashing. 0 means no
 * downsample
 * @param downsampleHeight Height to downsample to before hashing. 0 means no
 * downsample
 * @param num_threads Number of threads to use for hashing. 0 is auto.
 *
 * @return Video hashed successfully.
 *
 * @note If hashing fails for any reason the result will be empty.
 */
bool hashVideoFile(
    const std::string& inputVideoFileName,
    std::vector<hashing::vpdqFeature>& pdqHashes,
    bool verbose,
    const double secondsPerHash,
    const int downsampleWidth,
    const int downsampleHeight,
    const unsigned int num_threads);

} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // FILEHASHER_H
