cmake_minimum_required (VERSION 3.18)

project (FunctionBandit CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(
  CMAKE_CXX_FLAGS
  "-Werror -Wall -Wextra -Wpedantic -fno-exceptions"
)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set (CMAKE_CXX_STANDARD 17)

add_subdirectory (src)
