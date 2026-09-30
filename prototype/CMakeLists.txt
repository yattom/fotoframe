cmake_minimum_required(VERSION 3.25)
project(fotoframe_prototype CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TURBOJPEG REQUIRED IMPORTED_TARGET libturbojpeg)
pkg_check_modules(EXIF REQUIRED IMPORTED_TARGET libexif)

add_executable(prototype main.cpp)
target_compile_options(prototype PRIVATE -Wall -Wextra)
target_link_libraries(prototype PRIVATE SDL3::SDL3 PkgConfig::TURBOJPEG PkgConfig::EXIF)
