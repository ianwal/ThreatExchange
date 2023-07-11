// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#include <math.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vpdq/cpp/hashing/bufferhasher.h>
#include <vpdq/cpp/hashing/filehasher.h>
#include <vpdq/cpp/hashing/vpdqHashType.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libswscale/swscale.h>
}

using namespace std;

namespace facebook {
namespace vpdq {
namespace hashing {

/**
 * Get pdq hashes for selected frames every secondsPerHash
 * The return boolean represents whether the hashing process
 * is successful or not.
 **/

bool hashVideoFile(
    const string& inputVideoFileName,
    vector<hashing::vpdqFeature>& pdqHashes,
    const string& ffmpegPath,
    bool verbose,
    const double secondsPerHash,
    const int width,
    const int height,
    const double framesPerSec,
    const char* argv0) {
  std::unique_ptr<vpdq::hashing::AbstractFrameBufferHasher> phasher =
      vpdq::hashing::FrameBufferHasherFactory::createFrameHasher(height, width);
  if (phasher == nullptr) {
    fprintf(stderr, "Error: Phasher is null\n");
    return false;
  }

  // Open the input file
  AVFormatContext* formatContext = nullptr;
  if (avformat_open_input(
          &formatContext, inputVideoFileName.c_str(), nullptr, nullptr) != 0) {
    fprintf(stderr, "Error: Cannot open the video\n");
    return false;
  }

  // Retrieve stream information
  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    fprintf(stderr, "Error: Cannot find stream info\n");
    avformat_close_input(&formatContext);
    return false;
  }

  // Find the first video stream
  int videoStreamIndex = -1;
  for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
    if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStreamIndex = i;
      break;
    }
  }

  if (videoStreamIndex == -1) {
    fprintf(stderr, "Error: No video stream found\n");
    avformat_close_input(&formatContext);
    return false;
  }

  // Get the video codec parameters
  AVCodecParameters* codecParameters =
      formatContext->streams[videoStreamIndex]->codecpar;

  // Find the video decoder
  const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
  if (!codec) {
    fprintf(stderr, "Error: Codec decoder not found\n");
    avformat_close_input(&formatContext);
    return false;
  }

  // Create the codec context
  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
    fprintf(stderr, "Error: Failed to copy codec parameters to context\n");
    avformat_close_input(&formatContext);
    return false;
  }

  // Determine the number of threads to use and multithreading type
  codecContext->thread_count = 0;

  if (codec->capabilities & AV_CODEC_CAP_FRAME_THREADS) {
    codecContext->thread_type = FF_THREAD_FRAME;
  } else if (codec->capabilities & AV_CODEC_CAP_SLICE_THREADS) {
    codecContext->thread_type = FF_THREAD_SLICE;
  } else {
    codecContext->thread_count = 1;
  }

  // Open the codec context
  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    fprintf(stderr, "Error: Failed to open codec\n");
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  // Create the output frame
  AVFrame* frame = av_frame_alloc();

  // Pixel format for the image passed to PDQ
  constexpr AVPixelFormat pixelFormat = AV_PIX_FMT_RGB24;

  // Create the target frame for resizing
  AVFrame* targetFrame = av_frame_alloc();
  targetFrame->format = pixelFormat;
  targetFrame->width = width;
  targetFrame->height = height;
  av_image_alloc(
      targetFrame->data, targetFrame->linesize, width, height, pixelFormat, 1);

  // Allocate buffer for target frame
  int numBytes = av_image_get_buffer_size(
      pixelFormat, targetFrame->width, targetFrame->height, 1);
  uint8_t* buffer = (uint8_t*)av_malloc(numBytes);
  av_image_fill_arrays(
      targetFrame->data,
      targetFrame->linesize,
      buffer,
      pixelFormat,
      targetFrame->width,
      targetFrame->height,
      1);

  // Create the image rescaler context
  SwsContext* swsContext = sws_getContext(
      codecContext->width,
      codecContext->height,
      codecContext->pix_fmt,
      width,
      height,
      pixelFormat,
      SWS_LANCZOS,
      nullptr,
      nullptr,
      nullptr);

  AVPacket* packet = av_packet_alloc();

  int fno = 0;
  int frameMod = secondsPerHash * framesPerSec;
  if (frameMod == 0) {
    // Avoid truncate to zero on corner-case with secondsPerHash = 1
    // and framesPerSec < 1.
    frameMod = 1;
  }

  // Read frames in a loop and process them
  bool failed = false;
  while (av_read_frame(formatContext, packet) == 0) {
    // Check if the packet belongs to the video stream
    if (packet->stream_index == videoStreamIndex) {
      // Send the packet to the decoder
      if (avcodec_send_packet(codecContext, packet) < 0) {
        fprintf(stderr, "Error: Cannot send packet to decoder\n");
        failed = true;
        goto cleanup;
      }

      // Process the frame at interval
      if (fno % frameMod == 0) {
        // Receive the decoded frame
        while (avcodec_receive_frame(codecContext, frame) == 0) {
          // Resize the frame and convert to RGB24
          sws_scale(
              swsContext,
              frame->data,
              frame->linesize,
              0,
              codecContext->height,
              targetFrame->data,
              targetFrame->linesize);
          // Call pdqHasher to hash the frame
          int quality;
          pdq::hashing::Hash256 pdqHash;
          if (!phasher->hashFrame(targetFrame->data[0], pdqHash, quality)) {
            fprintf(
                stderr,
                "%s: failed to hash frame buffer. Frame width or height smaller than the minimum hashable dimension. %d.\n",
                argv0,
                fno);
            failed = true;
            goto cleanup;
          }

          // Push to pdqHashes vector
          pdqHashes.push_back(
              {pdqHash, fno, quality, (double)fno / framesPerSec});
          if (verbose) {
            printf("PDQHash: %s\n", pdqHash.format().c_str());
          }
          fno += 1;
        }
      }
    }

    av_packet_unref(packet);
  }

  // Flush the decoder to drain all buffered frames
  while (avcodec_send_packet(codecContext, packet) != AVERROR_EOF) {
    if (fno % frameMod == 0) {
      // Receive the decoded frame
      while (avcodec_receive_frame(codecContext, frame) == 0) {
        // Resize the frame and convert to RGB24
        sws_scale(
            swsContext,
            frame->data,
            frame->linesize,
            0,
            codecContext->height,
            targetFrame->data,
            targetFrame->linesize);
        // Call pdqHasher to hash the frame
        int quality;
        pdq::hashing::Hash256 pdqHash;
        if (!phasher->hashFrame(targetFrame->data[0], pdqHash, quality)) {
          fprintf(
              stderr,
              "%s: failed to hash frame buffer. Frame width or height smaller than the minimum hashable dimension. %d.\n",
              argv0,
              fno);
          failed = true;
          goto cleanup;
        }

        // Add vpdqFeature to vector
        pdqHashes.push_back(
            {pdqHash, fno, quality, (double)fno / framesPerSec});
        if (verbose) {
          printf("PDQHash: %s \n", pdqHash.format().c_str());
        }
        fno += 1;
      }
    }
    av_packet_unref(packet);
  }

cleanup:
  av_packet_free(&packet);
  sws_freeContext(swsContext);
  av_frame_free(&frame);
  av_frame_free(&targetFrame);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);

  if (failed) {
    return false;
  }

  return true;
}

} // namespace hashing
} // namespace vpdq
} // namespace facebook
