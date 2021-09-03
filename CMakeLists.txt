cmake_minimum_required(VERSION 3.20)
project(ennel)

# This would be C17, but CMake v3.20 has forced my hand.
# Once I can use CMake v3.21 or higher, this will be updated.
set(CMAKE_C_STANDARD 11)

include_directories(include/)
file(GLOB_RECURSE ennel_src src/*.c)
add_executable(ennel ${ennel_src})
