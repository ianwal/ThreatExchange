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
  AbstractFrameBufferHasher(int frameHeight, int frameWidth);

  virtual ~AbstractFrameBufferHasher() = default;

  // Number of floats in each framewise hash
  virtual int getFeatureDimension() const = 0;

  // Get PDQ Hash in Hash256 format
  virtual bool hashFrame(
      unsigned char* buffer,
      facebook::pdq::hashing::Hash256& hash,
      int& quality) = 0;

 protected:
  int _frameHeight;
  int _frameWidth;
  int _numRGBTriples;
};

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

// A factory design pattern to create the Buffer Hasher
class FrameBufferHasherFactory {
 public:
  static int getFrameHasherDownscaleDimension();
  static std::unique_ptr<AbstractFrameBufferHasher> createFrameHasher(
      int frameHeight, int frameWidth);
};

} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // BUFFERHASHER_H
