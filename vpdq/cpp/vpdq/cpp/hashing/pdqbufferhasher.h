// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#ifndef PDQBUFFERHASHER_H
#define PDQBUFFERHASHER_H

#include <vector>

#include <pdq/cpp/common/pdqhashtypes.h>
#include <vpdq/cpp/hashing/bufferhasher.h>

namespace facebook {
namespace vpdq {
namespace hashing {

/**
 *
 * PDQ Hash FrameBufferHasher class which inherits AbstractFrameBufferHasher
 *
 * @param frameHeight Input frame's height
 * @param frameWidth Input frame's height
 *
 */
class PDQFrameBufferHasher : public AbstractFrameBufferHasher {
 public:
  PDQFrameBufferHasher(int frameHeight, int frameWidth);

  ~PDQFrameBufferHasher() = default;

  int getFeatureDimension() const override;

  bool hashFrame(
      unsigned char* buffer,
      facebook::pdq::hashing::Hash256& hash,
      int& quality) override;

  static constexpr int getFrameDownscaleDimension() { return 64; }

 private:
  // Variables for computing pdq hash
  std::vector<float> _fullLumaImageBuffer1;
  std::vector<float> _fullLumaImageBuffer2;
  float _buffer64x64[64][64];
  float _buffer16x64[16][64];
  float _buffer16x16[16][16];
};

} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // PDQBUFFERHASHER_H
