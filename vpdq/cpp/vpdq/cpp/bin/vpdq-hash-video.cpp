// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#include <cstdlib>
#include <string>
#include <vector>

#include <vpdq/cpp/hashing/filehasher.h>
#include <vpdq/cpp/hashing/vpdqHashType.h>
#include <vpdq/cpp/io/vpdqio.h>

namespace {

void usage(char* argv0, int rc) {
  FILE* fp = (rc == 0) ? stdout : stderr;
  std::fprintf(fp, "Usage: %s [options]\n\n", argv0);
  std::fprintf(fp, "Required:\n\n");
  std::fprintf(fp, "-i|--input-video-file-name ...\n\n");
  std::fprintf(fp, "-o|--output-hash-file-name ...\n\n");
  std::fprintf(
      fp,
      "-r|--seconds-per-hash ...:Must be a non-negative float. If it is 0, will generate every frame's hash\n\n");
  std::fprintf(fp, "Options:\n\n");
  std::fprintf(fp, "-v|--verbose: Show all hash matching information\n\n");
  std::fprintf(
      fp,
      "-d|--output-directory ...: instead of specifying "
      "output-file name, just give the directory and the output file name will "
      "be computed from the input video file name. Example: avideofile.mp4 -> output_directory>/avideofile.txt\n\n");
  std::fprintf(
      fp,
      "-s|--downsample-frame-dimension ...: The down scaling resolution for video. The input number will be the height and width of the downscaled video. For example, -s 160 -> will make video of 1080x720 to 160x160.\n\n");
  std::exit(rc);
}

/**
 *
 * Get the base name with extension of the input path.
 *
 * Example: ./dir/sub-dir/sample.txt -> sample.txt
 *
 * @param filename Path of the file.
 *
 * @return Filename without the path.
 * 
 */
std::string basename(const std::string& filename) {
  auto const separator_pos = filename.find_last_of("\\/");
  auto const base_name = (separator_pos == std::string::npos)
      ? filename
      : filename.substr(separator_pos + 1);
  return base_name;
}

/**
 *
 * Strip the extension of the input filename.
 *
 * Example: sample.txt -> sample
 *
 * @param filename Path of the file.
 *
 * @return Filename without the last extension.
 * 
 */
std::string stripExtension(const std::string& filename) {
  auto const delimiter_pos = filename.rfind('.');
  auto const name = (delimiter_pos == std::string::npos)
      ? filename
      : filename.substr(0, delimiter_pos);
  return name;
}

} // namespace

int main(int argc, char** argv) {
  int argi = 1;
  bool verbose = false;
  std::string inputVideoFileName{};
  std::string outputHashFileName{};
  std::string outputDirectory{};
  double secondsPerHash = 0;
  int downsampleFrameDimension = 0;
  unsigned int thread_count = 0;

  while ((argi < argc) && argv[argi][0] == '-') {
    const std::string flag(argv[argi]);
    ++argi;
    if (flag == "-v" || flag == "--verbose") {
      verbose = true;
    }
    else if (flag == "-i" || flag == "--input-video-file-name") {
      if ((argc - argi) < 1) {
        usage(argv[0], 1);
      }
      inputVideoFileName = std::string(argv[argi]);
      ++argi;
    }
    else if (flag == "-o" || flag == "--output-hash-file-name") {
      if ((argc - argi) < 1) {
        usage(argv[0], 1);
      }
      outputHashFileName = std::string(argv[argi]);
      ++argi;
    }
    else if (flag == "-r" || flag == "--seconds-per-hash") {
      if ((argc - argi) < 1) {
        usage(argv[0], 1);
      }
      secondsPerHash = std::atof(argv[argi]);
      ++argi;
    }
    else if (flag == "-d" || flag == "--output-directory") {
      if ((argc - argi) < 1) {
        usage(argv[0], 1);
      }
      outputDirectory = std::string(argv[argi]);
      ++argi;
    }
    else if (flag == "-s" || flag == "--downsample-frame-dimension") {
      if ((argc - argi) < 1) {
        usage(argv[0], 1);
      }
      downsampleFrameDimension = std::atoi(argv[argi]);
      ++argi;
    }
    else if (flag == "-t" || flag == "--thread-count") {
      if ((argc - argi) < 1) {
        usage(argv[0], 1);
      }
      thread_count = std::atoi(argv[argi]);
      ++argi;
    }
    else
    {
      usage(argv[0], 1);
    }
  }

  if (inputVideoFileName.empty()) {
    std::fprintf(stderr, "%s: --input-video-file-name missing\n", argv[0]);
    usage(argv[0], 1);
  }

  if ((outputHashFileName.empty() && outputDirectory.empty()) ||
      (!outputHashFileName.empty() && !outputDirectory.empty())) {
    std::fprintf(
        stderr,
        "%s: need one of --output-hash-file-name "
        "or --output-directory\n",
        argv[0]);
    usage(argv[0], 1);
  }

  if (secondsPerHash < 0) {
    std::fprintf(
        stderr,
        "%s: --seconds-per-hash must be a non-negative float.\n",
        argv[0]);
    usage(argv[0], 1);
  }

  // Get the output hash file name if outputDirectory is specified
  if (!outputDirectory.empty()) {
    auto const base_name = stripExtension(basename(inputVideoFileName));
    outputHashFileName = outputDirectory + "/" + base_name + ".txt";
  }

  // Hash the video and store the features in pdqHashes
  std::vector<facebook::vpdq::hashing::vpdqFeature> pdqHashes;
  if (!facebook::vpdq::hashing::hashVideoFile(
      inputVideoFileName,
      pdqHashes,
      verbose,
      secondsPerHash,
      downsampleFrameDimension,
      downsampleFrameDimension,
      thread_count)) {
    std::fprintf(
        stderr,
        "%s: failed to hash \"%s\".\n",
        argv[0],
        inputVideoFileName.c_str());
    return EXIT_FAILURE;
  }

  // Write all vpdq features to the output file.
  if (!facebook::vpdq::io::outputVPDQFeatureToFile(outputHashFileName, pdqHashes))
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
