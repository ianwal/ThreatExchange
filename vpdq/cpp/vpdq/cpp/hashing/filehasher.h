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
 * Hash a video file using the vPDQ algorithm.
 *
 * @param[in] inputVideoFileName Input video file path.
 * @param[in, out] vpdqFeatures  The collection of hashed frames.
 *
 * @return Video hashed successfully or not.
 *
 * @note If hashing fails for any reason, the result will be empty.
 */
bool hashVideoFile(
    const std::string& inputVideoFileName,
    std::vector<hashing::vpdqFeature>& vpdqFeatures);

/**
 * Hash a video file using the vPDQ algorithm.
 *
 * @param[in]      inputVideoFileName Input video file path.
 * @param[in, out] vpdqFeatures       The collection of hashed frames.
 * @param[in]      verbose            If produce detailed output for diagnostic
 *                                    purposes.
 * @param[in]      secondsPerHash     The time period of picking frames in vpdq.
 * @param[in]      downsampleWidth    Width to downsample frames to for hashing.
 * @param[in]      downsampleHeight   Height to downsample frames to for hashing.
 * @param[in]      num_threads        Number of threads to use for hashing.
 *
 * @return Video hashed successfully or not.
 *
 * @note If hashing fails for any reason, the result will be empty.
 * @note If the downsample dimensions are larger than the original video
 *       dimensions or <=0, the original video dimensions will be used.
 * @note If num_threads == 0, one thread will be spawned per CPU core.
 */
bool hashVideoFile(
    const std::string& inputVideoFileName,
    std::vector<hashing::vpdqFeature>& vpdqFeatures,
    bool verbose,
    const double secondsPerHash,
    const int downsampleWidth,
    const int downsampleHeight,
    const unsigned int num_threads);

} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // FILEHASHER_H
