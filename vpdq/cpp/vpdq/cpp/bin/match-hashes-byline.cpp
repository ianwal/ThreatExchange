// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>

#include <pdq/cpp/io/hashio.h>
#include <vpdq/cpp/hashing/vpdqHashType.h>
#include <vpdq/cpp/io/vpdqio.h>

namespace {

void usage(char* argv0, int rc) {
  FILE* fp = (rc == 0) ? stdout : stderr;
  std::fprintf(
      fp,
      "Usage: %s [options] file1name file2name hamming_distanceTolerance qualityTolerance\n\n",
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

  using facebook::vpdq::hashing::vpdqFeature;
  // Load video 1 hashes.
  std::vector<vpdqFeature> video1Hashes;
  if (!facebook::vpdq::io::loadHashesFromFileOrDie(argv[argi], video1Hashes)) {
    return EXIT_FAILURE;
  }

  // Load video 2 hashes.
  std::vector<vpdqFeature> video2Hashes;
  if (!facebook::vpdq::io::loadHashesFromFileOrDie(
          argv[argi + 1], video2Hashes)) {
    return EXIT_FAILURE;
  }

  const auto distanceTolerance = std::atoi(argv[argi + 2]);
  const auto qualityTolerance = std::atoi(argv[argi + 3]);

  if (video1Hashes.size() != video2Hashes.size()) {
    std::cerr << "VideoHashes1 size " << video1Hashes.size()
              << " doesn't match with VideoHashes2 size " << video2Hashes.size()
              << std::endl;
    return EXIT_FAILURE;
  }

  size_t match_count = 0;
  size_t total_compared = 0;
  for (size_t i = 0; i < video1Hashes.size(); ++i) {
    const auto& hash1 = video1Hashes[i];
    const auto& hash2 = video2Hashes[i];

    if (hash1.quality < qualityTolerance || hash2.quality < qualityTolerance) {
      if (verbose) {
        std::cout << "Skipping Line " << i
                  << " Hash1: " << hash1.pdqHash.format()
                  << " Hash2: " << hash2.pdqHash.format()
                  << ", because of low quality. Hash1: " << hash1.quality
                  << " Hash2: " << hash2.quality << '\n';
      }
    } else {
      if (hash1.pdqHash.hammingDistance(hash2.pdqHash) < distanceTolerance) {
        ++match_count;
        if (verbose) {
          std::cout << "Line " << i << " Hash1: " << hash1.pdqHash.format()
                    << " Hash2: " << hash2.pdqHash.format() << " match" << '\n';
        }
      } else {
        if (verbose) {
          std::cout << "NO MATCH: Line " << i
                    << " Hash1: " << hash1.pdqHash.format()
                    << " Hash2: " << hash2.pdqHash.format() << '\n';
        }
      }

      ++total_compared;
    }
  }

  auto const match_percentage =
      static_cast<float>(match_count) * 100 / total_compared;
  std::cout << std::fixed << std::setprecision(3) << match_percentage
            << " Percentage matches" << '\n';
  return EXIT_SUCCESS;
}
