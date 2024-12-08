cmake_minimum_required(VERSION 3.10)
project("Project1-DucHDM-225179")

set(CMAKE_CXX_STANDARD 14)

set(CUSTOM_ZIPLIB src/ZipBitStream.cpp src/ZipCentralDirectory.cpp src/ZipDeflate.cpp src/ZipEndOfCentralDirectoryRecord.cpp src/ZipFile.cpp src/ZipLocalFile.cpp src/ZipUtil.cpp)
include_directories(include)

add_executable(ziptool src/ziptool.cpp ${CUSTOM_ZIPLIB})
add_executable(benchmark src/benchmark.cpp ${CUSTOM_ZIPLIB})