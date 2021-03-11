# CMakeLists.txt

# cmake
cmake_minimum_required(VERSION 3.5)

# CFLAGS, CXXFLAGS
add_compile_options(
# it would be nice to be able to define variant here, but this triggers
# ESPmDNS bug (so do it in sketch.h instead):
# -DARDUINO_BOARD=FEATHER_ESP32
# -DARDUINO_VARIANT=feather_esp32
# -DARDUINO_FEATHER_ESP32

  $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-fallthrough>
  $<$<COMPILE_LANGUAGE:C>:-Wno-deprecated-declarations>

  $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-maybe-uninitialized>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-all>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-int-to-pointer-cast>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-function>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-variable>
)

# arduino core
set(EXTRA_COMPONENT_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/local-sdks/arduino-esp32
)

# IDF
set(PROJECT_VER "1.0.0")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(IFTTTThing)
