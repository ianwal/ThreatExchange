// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#include <memory>
#include <string>
#include <vector>

#include <pdq/cpp/common/pdqhashtypes.h>
#include <pdq/cpp/downscaling/downscaling.h>
#include <pdq/cpp/hashing/pdqhashing.h>
#include <vpdq/cpp/hashing/bufferhasher.h>

namespace facebook {
namespace vpdq {
namespace hashing {

// ----------------------------------------------------------------
bool PDQFrameBufferHasher::hashFrame(
    unsigned char* buffer, // The PDQ hash buffer
    pdq::hashing::Hash256& hash, // The result pdq hash
    int& quality // Hashing Quality
) {
  constexpr int MIN_HASHABLE_DIM{5};

  // If the frame dimensions are too small, clear the hash and return early.
  if (_frameHeight < MIN_HASHABLE_DIM || _frameWidth < MIN_HASHABLE_DIM) {
    hash.clear();
    quality = 0;
    return false;
  }

  pdq::hashing::fillFloatLumaFromRGB(
      &buffer[0], // pRbase
      &buffer[1], // pGbase
      &buffer[2], // pBbase
      _frameHeight,
      _frameWidth,
      3 * _frameWidth, // rowStride
      3, // colStride
      _fullLumaImageBuffer1.data());

  // PDQ Buffers
  float _buffer64x64[64][64];
  float _buffer16x64[16][64];
  float _buffer16x16[16][16];

  pdq::hashing::pdqHash256FromFloatLuma(
      _fullLumaImageBuffer1.data(), // numRows x numCols, row-major
      _fullLumaImageBuffer2.data(), // numRows x numCols, row-major
      _frameHeight,
      _frameWidth,
      _buffer64x64,
      _buffer16x64,
      _buffer16x16,
      hash,
      quality);

  return true;
}

namespace FrameBufferHasherFactory {

// ----------------------------------------------------------------
std::unique_ptr<AbstractFrameBufferHasher> createFrameHasher(
    const int frameHeight, const int frameWidth) {
  return std::make_unique<PDQFrameBufferHasher>(frameHeight, frameWidth);
}

} // namespace FrameBufferHasherFactory

} // namespace hashing
} // namespace vpdq
} // namespace facebook
