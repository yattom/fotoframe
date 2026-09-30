cmake_minimum_required(VERSION 3.25)
project(fotoframe CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# ロジック部分。SDL やファイル以外の入出力に依存せず、ユニットテストの対象にする。
add_library(fotoframe_core STATIC
    src/photo_filter.cpp
    src/photo_scanner.cpp
    src/photo_deck.cpp
    src/photo_buffer.cpp
    src/sleep_schedule.cpp
    src/decode_scale.cpp
    src/layout.cpp
    src/transition.cpp
    src/presentation.cpp
    src/config.cpp
    src/slideshow.cpp
)
target_include_directories(fotoframe_core PUBLIC src)
find_package(tomlplusplus REQUIRED)
target_link_libraries(fotoframe_core PRIVATE tomlplusplus::tomlplusplus)

# 外部（SDL3、libturbojpeg、libexif、wlopm）とのやり取りを含む本体。動かして確認する。
find_package(SDL3 REQUIRED CONFIG)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TURBOJPEG REQUIRED IMPORTED_TARGET libturbojpeg)
pkg_check_modules(EXIF REQUIRED IMPORTED_TARGET libexif)
add_executable(fotoframe
    src/main.cpp
    src/jpeg_decoder.cpp
    src/renderer.cpp
    src/display_power.cpp
    src/loader.cpp
)
target_link_libraries(fotoframe PRIVATE fotoframe_core SDL3::SDL3 PkgConfig::TURBOJPEG PkgConfig::EXIF)

enable_testing()
find_package(GTest REQUIRED)
add_executable(fotoframe_tests
    tests/photo_filter_test.cpp
    tests/photo_scanner_test.cpp
    tests/photo_deck_test.cpp
    tests/photo_buffer_test.cpp
    tests/sleep_schedule_test.cpp
    tests/decode_scale_test.cpp
    tests/layout_test.cpp
    tests/transition_test.cpp
    tests/presentation_test.cpp
    tests/config_test.cpp
    tests/slideshow_test.cpp
)
target_link_libraries(fotoframe_tests PRIVATE fotoframe_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(fotoframe_tests)
