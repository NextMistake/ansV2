cmake_minimum_required(VERSION 3.7)
project(ansV2)

set(CMAKE_CXX_STANDARD 11)

include_directories(src)

set(SOURCE_FILES src/main.cpp src/ans_Config.h src/ans_Log.h src/ans_File.h src/ans_Recombine.h src/nlohann_json.hpp src/ans_Config.cpp src/ans_Log.cpp)
add_executable(ansV2 ${SOURCE_FILES})