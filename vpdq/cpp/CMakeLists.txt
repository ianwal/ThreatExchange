cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_FLAGS "-O2 -fPIC -Wall -Wextra -Werror -Wno-unused-function -Wno-deprecated-declarations")
project(VPDQ)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

set(CMAKE_CXX_STANDARD 14)

set (PDQSOURCES
    ./pdq/cpp/common/pdqhashtypes.cpp
    ./pdq/cpp/hashing/pdqhashing.cpp
    ./pdq/cpp/common/pdqhamming.cpp
    ./pdq/cpp/io/hashio.cpp
    ./pdq/cpp/downscaling/downscaling.cpp
    ./pdq/cpp/hashing/torben.cpp
)

set (PDQHEADERS
    ./pdq/cpp/common/pdqhashtypes.h
    ./pdq/cpp/common/pdqbasetypes.h
    ./pdq/cpp/common/pdqhamming.h
    ./pdq/cpp/hashing/pdqhashing.h
    ./pdq/cpp/io/hashio.h
    ./pdq/cpp/downscaling/downscaling.h
    ./pdq/cpp/hashing/torben.h
)

set(SOURCES
    ./hashing/bufferhasher.cpp
    ./hashing/filehasher.cpp
    ./hashing/matchTwoHash.cpp
    ./io/vpdqio.cpp
)

SET(HEADERS
    ./hashing/bufferhasher.h
    ./hashing/filehasher.h
    ./hashing/vpdqHashType.h
    ./hashing/matchTwoHash.h
    ./io/vpdqio.h
)

include_directories( ../bin ../hashing ../io ${CMAKE_SOURCE_DIR} )

add_library(vpdq
    ${SOURCES}
    ${HEADERS}
    ${PDQSOURCES}
    ${PDQHEADERS}
)

target_include_directories(vpdq PRIVATE ${CMAKE_SOURCE_DIR})

add_executable(match-hashes-byline
    bin/match-hashes-byline.cpp
)

add_executable(match-hashes-brute
    bin/match-hashes-brute.cpp
)

add_executable(vpdq-hash-video
    bin/vpdq-hash-video.cpp
)


target_link_libraries(vpdq
    PkgConfig::LIBAV
)

target_link_libraries(match-hashes-byline
    vpdq
    PkgConfig::LIBAV
)

target_link_libraries(match-hashes-brute
    vpdq
    PkgConfig::LIBAV
)

target_link_libraries(vpdq-hash-video
    vpdq
    PkgConfig::LIBAV
)

# Write the library dirs to a file for Cython to find the LIBAV files
string(REPLACE ";" "\n" LIBRARY_DIRS "${LIBAV_STATIC_LIBRARY_DIRS}")
set(LIBRARY_DIRS_FILE "libraries-dirs.txt")
file(WRITE ${LIBRARY_DIRS_FILE} "${LIBRARY_DIRS}")
