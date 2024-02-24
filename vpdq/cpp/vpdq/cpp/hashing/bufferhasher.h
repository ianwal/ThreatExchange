// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#ifndef BUFFERHASHER_H
#define BUFFERHASHER_H

#include <memory>
#include <string>
#include <vector>
#include <pdq/cpp/common/pdqhashtypes.h>
#include <pdq/cpp/hashing/pdqhashing.h>

namespace facebook {
namespace vpdq {
namespace hashing {

/**
 *
 * Abstract FrameBufferHasher class
 *
 * @param frameHeight Input frame's height
 * @param frameWidth Input frame's width
 *
 */
class AbstractFrameBufferHasher {
 public:
  AbstractFrameBufferHasher(int frameHeight, int frameWidth)
      : _frameHeight(frameHeight), _frameWidth(frameWidth) {}

  virtual ~AbstractFrameBufferHasher() = default;

  // Number of floats in each framewise hash
  virtual int getFeatureDimension() = 0;

  virtual bool hashFrame(
      unsigned char* buffer, pdq::hashing::Hash256& hash, int& quality) = 0;

 protected:
  int _frameHeight;
  int _frameWidth;
};

/**
 *
 * PDQ Hash FrameBufferHasher class which inherits AbstractFrameBufferHasher
 *
 * @param frameHeight Input frame height
 * @param frameWidth Input frame height
 *
 */
class PDQFrameBufferHasher : public AbstractFrameBufferHasher {
 public:
  PDQFrameBufferHasher(int frameHeight, int frameWidth)
      : AbstractFrameBufferHasher(frameHeight, frameWidth) {
    const int numRGBTriples{frameHeight * frameWidth};
    _fullLumaImageBuffer1 = std::vector<float>(numRGBTriples);
    _fullLumaImageBuffer2 = std::vector<float>(numRGBTriples);
  }

  ~PDQFrameBufferHasher() = default;

  static int getFrameDownscaleDimension() { return SCALED_DIMENSION; }

  int getFeatureDimension() override { return pdq::hashing::HASH256_NUM_BITS; }

  // Get PDQ Hash in Hash256 format
  bool hashFrame(
      unsigned char* buffer,
      pdq::hashing::Hash256& hash,
      int& quality) override;

 private:
  // Variables for computing pdq hash
  std::vector<float> _fullLumaImageBuffer1;
  std::vector<float> _fullLumaImageBuffer2;
  static constexpr int SCALED_DIMENSION = 64;
};

// A factory design pattern to create the Buffer Hasher
namespace FrameBufferHasherFactory {

int getFrameHasherDownscaleDimension();

std::unique_ptr<AbstractFrameBufferHasher> createFrameHasher(
    int frameHeight, int frameWidth);

} // namespace FrameBufferHasherFactory

} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // BUFFERHASHER_H
