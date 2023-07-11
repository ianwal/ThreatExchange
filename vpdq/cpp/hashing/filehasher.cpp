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
 * Get frames by passing video file through ffmpeg
 * Then get pdq hashes for selected frames every secondsPerHash
 * The return boolean represents whether the hashing process is successful or
 *not.
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
    fprintf(stderr, "Error: Decoder not found\n");
    avformat_close_input(&formatContext);
    return false;
  }

  // Create the codec context and open it
  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
    fprintf(stderr, "Error: Failed to copy codec parameters to context\n");
    avformat_close_input(&formatContext);
    return false;
  }

  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    fprintf(stderr, "Error: Failed to open codec\n");
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  // Create the output frame
  AVFrame* frame = av_frame_alloc();

  // Create the target frame for resizing
  AVFrame* targetFrame = av_frame_alloc();
  targetFrame->format = AV_PIX_FMT_RGB24;
  targetFrame->width = width;
  targetFrame->height = height;
  av_image_alloc(
      targetFrame->data,
      targetFrame->linesize,
      width,
      height,
      codecContext->pix_fmt,
      1);

  // Calculate bytes per pixel
  int bytesPerPixel = av_get_bytes_per_sample(
      static_cast<AVSampleFormat>(codecContext->pix_fmt));

  // Allocate buffer for target frame
  int numBytes =
      av_image_get_buffer_size(codecContext->pix_fmt, width, height, 1);
  uint8_t* buffer =
      static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));
  av_image_fill_arrays(
      targetFrame->data,
      targetFrame->linesize,
      buffer,
      codecContext->pix_fmt,
      width,
      height,
      1);

  // Create the image rescaler context
  SwsContext* swsContext = sws_getContext(
      codecContext->width,
      codecContext->height,
      codecContext->pix_fmt,
      width,
      height,
      AV_PIX_FMT_RGB24,
      SWS_BILINEAR,
      nullptr,
      nullptr,
      nullptr);

  // Seek to the beginning of the video
  av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);

  // Variables for one-second intervals
  int64_t nextFrameTime = 0;
  int64_t frameTimeIncrement = static_cast<int64_t>(
      AV_TIME_BASE * formatContext->streams[videoStreamIndex]->time_base.den /
      formatContext->streams[videoStreamIndex]->time_base.num);

  std::cout << frameTimeIncrement << endl;

  AVPacket* packet = av_packet_alloc();

  int fno = 0;
  AVRational fr = formatContext->streams[videoStreamIndex]->avg_frame_rate;
  int frameMod = secondsPerHash * framesPerSec;
  printf("%d %f\n", frameMod, framesPerSec);
  if (frameMod == 0) {
    // Avoid truncate to zero on corner-case with secondsPerHash = 1
    // and framesPerSec < 1.
    frameMod = 1;
  }

  // Read frames
  while (av_read_frame(formatContext, packet) >= 0) {
        // Process the frame at interval
    // Check if the packet belongs to the video stream
    if (packet->stream_index == videoStreamIndex) {
      // Send the packet to the decoder
      if (avcodec_send_packet(codecContext, packet) < 0) {
        fprintf(stderr, "Error: Cannot send packet to decoder\n");
        break;
      }

      // Receive the decoded frame
      while (avcodec_receive_frame(codecContext, frame) == 0) {
        if (fno % frameMod == 0) {
            if (frame->format != AV_PIX_FMT_RGB24) {
                sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height,
                          targetFrame->data, targetFrame->linesize);
            } else {
                // The frame is already in RGB24 format
                av_frame_copy(targetFrame, frame);
            }
        // Resize the frame
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
          printf("%d %d %d\n", targetFrame->data[0][0], targetFrame->data[0][1], targetFrame->data[0][2]);
          if (!phasher->hashFrame(targetFrame->data[0], pdqHash, quality)) {
            fprintf(
                stderr,
                "%s: failed to hash frame buffer. Frame width or height smaller than the minimum hashable dimension. %d.\n",
                argv0,
                fno);
            break;
          }


          // Increment the frame number counter

          // Push to pdqHashes vector
          pdqHashes.push_back(
              {pdqHash, fno, quality, (double)fno / framesPerSec});
          if (verbose) {
            printf("PDQHash: %s \n", pdqHash.format().c_str());
          }

          // For demonstration purposes, print the frame's width and height
          std::cout << "Frame: " << fno << " Frame Width: " << targetFrame->width
                    << ", Height: " << targetFrame->height << std::endl;

          // Increment the next frame time by one second
          //nextFrameTime += frameTimeIncrement;
        }
      fno += 1;
      }
    }

    av_packet_unref(packet);
  }

  av_frame_free(&frame);
  av_frame_free(&targetFrame);
  avcodec_close(codecContext);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);
  sws_freeContext(swsContext);

  return true;
/*
  stringstream ss;

  ss << quoted(inputVideoFileName);
  string escapedInputVideoFileName = ss.str();
  // FFMPEG command to process the downsampled video

  string ffmpegLogLevel =
      verbose ? "" : "-loglevel error -hide_banner -nostats";
  string command = ffmpegPath + " " + ffmpegLogLevel + " -nostdin -i " +
      escapedInputVideoFileName + " -s " + to_string(width) + ":" +
      to_string(height) + " -an -f rawvideo -c:v rawvideo -pix_fmt rgb24" +
      " pipe:1";
  FILE* inputFp = popen(command.c_str(), "r");
  if (inputFp == nullptr) {
    fprintf(stderr, "%s: ffmpeg to generate video stream failed\n", argv0);
    return false;
  }

  bool eof = false;

  // Create the PDQ Frame Buffer Hasher
  std::unique_ptr<vpdq::hashing::AbstractFrameBufferHasher> phasher =
      vpdq::hashing::FrameBufferHasherFactory::createFrameHasher(height, width);
  if (phasher == nullptr) {
    fprintf(stderr, "Error: Phasher is null");
    return false;
  }

  // Create a Frame Buffer to reuse everytime for hashing
  int numRGBTriples = height * width;
  int fno = 0;
  unique_ptr<uint8_t[]> rawFrameBuffer(new uint8_t[numRGBTriples * 3]);
  // Intentional floor operation calculate frameMod as an integer
  int frameMod = secondsPerHash * framesPerSec;
  if (frameMod == 0) {
    // Avoid truncate to zero on corner-case with secondsPerHash = 1
    // and framesPerSec < 1.
    frameMod = 1;
  }
  // Loop through the video frames
  while (!feof(inputFp)) {
    size_t fread_rc = fread(rawFrameBuffer.get(), 3, numRGBTriples, inputFp);
    if (fread_rc == 0) {
      eof = true;
    }
    if (eof) {
      break;
    }
    pdq::hashing::Hash256 pdqHash;
    if (fno % frameMod == 0) {
      if (verbose) {
        printf("selectframe %d\n", fno);
      }
      // Call pdqHasher to hash the frame
      int quality;
      if (!phasher->hashFrame(rawFrameBuffer.get(), pdqHash, quality)) {
        fprintf(
            stderr,
            "%s: failed to hash frame buffer. Frame width or height smaller than minimum hashable dimension. %d.\n",
            argv0,
            fno);
        return false;
      }
      printf("%d %d %d\n", rawFrameBuffer.get()[0], rawFrameBuffer.get()[1], rawFrameBuffer.get()[2]);
      // Push to pdqHashes vector
      pdqHashes.push_back({pdqHash, fno, quality, (double)fno / framesPerSec});
      if (verbose) {
        printf("PDQHash: %s \n", pdqHash.format().c_str());
      }
    }
    fno++;
    if (fread_rc != numRGBTriples) {
      perror("fread");
      fprintf(
          stderr,
          "Expected %d RGB triples; got %d\n",
          numRGBTriples,
          (int)fread_rc);
    }
  }
  return true;
 */ 
}

} // namespace hashing
} // namespace vpdq
} // namespace facebook
