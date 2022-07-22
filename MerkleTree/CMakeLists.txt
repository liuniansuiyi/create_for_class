cmake_minimum_required(VERSION 3.21)
project(MerkleTree)

set(CMAKE_CXX_STANDARD 14)
set(INC_DIR C:\\OpenSSL-Win64\\include)
set(LINK_DIR C:\\OpenSSL-Win64\\lib)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(ssl crypto)
add_executable(MerkleTree main.cpp)
