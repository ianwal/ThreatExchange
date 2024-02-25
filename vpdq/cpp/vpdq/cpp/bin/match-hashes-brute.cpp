// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#include <cstdio>
#include <cstring>

#include <pdq/cpp/io/hashio.h>
#include <vpdq/cpp/hashing/matchTwoHash.h>
#include <vpdq/cpp/hashing/vpdqHashType.h>
#include <vpdq/cpp/io/vpdqio.h>

namespace {

void usage(char* argv0, int rc) {
  FILE* fp = (rc == 0) ? stdout : stderr;
  std::fprintf(
      fp,
      "Usage: %s [options] queryFilename targetFilename hamming_distance_tolerance quality_tolerance\n\n",
      argv0);
  std::fprintf(fp, "Options:\n\n");
  std::fprintf(fp, "-v|--verbose: Show all hash matching information\n\n");
  std::exit(rc);
}

} // namespace

int main(int argc, char** argv) {
  int argi = 1;
  bool verbose = false;

  for (; argi < argc; argi++) {
    if (argv[argi][0] != '-') {
      break;
    }
    if (!std::strcmp(argv[argi], "-v") ||
        !std::strcmp(argv[argi], "--verbose")) {
      verbose = true;
      continue;
    }
  }

  if (argi > argc - 4) {
    usage(argv[0], 1);
  }

  const auto distanceTolerance = std::atoi(argv[argi + 2]);
  const auto qualityTolerance = std::atoi(argv[argi + 3]);

  // Load query hashes.
  std::vector<facebook::vpdq::hashing::vpdqFeature> qHashes;
  if (!facebook::vpdq::io::loadHashesFromFileOrDie(argv[argi], qHashes)) {
    return EXIT_FAILURE;
  }

  // Load target hashes.
  std::vector<facebook::vpdq::hashing::vpdqFeature> tHashes;
  if (!facebook::vpdq::io::loadHashesFromFileOrDie(argv[argi + 1], tHashes)) {
    return EXIT_FAILURE;
  }

  // Get video hash similarity.
  double qMatch = 0.0;
  double tMatch = 0.0;
  if (!facebook::vpdq::hashing::matchTwoHashBrute(
          qHashes,
          tHashes,
          distanceTolerance,
          qualityTolerance,
          qMatch,
          tMatch,
          verbose)) {
    return EXIT_FAILURE;
  }

  // Print float with 2 decimal places
  std::printf("%0.2f Percentage Query Video match\n", qMatch);
  std::printf("%0.2f Percentage Target Video match\n", tMatch);
  return EXIT_SUCCESS;
}
