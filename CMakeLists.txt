cmake_minimum_required(VERSION 3.7)
project(ansV2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -DANSV2_DEBUG")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

include_directories(src)

set(SOURCE_FILES src/main.cpp src/ans_Config.h src/ans_Log.h src/ans_File.h src/ans_Recombine.h src/nlohann_json.hpp src/ans_Config.cpp src/ans_Log.cpp src/ans.h src/ans.cpp src/ans_File.cpp)
add_executable(ansV2 ${SOURCE_FILES})

target_link_libraries(ansV2 pthread)