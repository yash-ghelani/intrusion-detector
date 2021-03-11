# CMakeLists.txt

FILE(GLOB_RECURSE COMPONENT_SRCS
  ${CMAKE_SOURCE_DIR}/main/*.h
  ${CMAKE_SOURCE_DIR}/main/*.c
  ${CMAKE_SOURCE_DIR}/main/*.cpp
)

set(COMPONENT_ADD_INCLUDEDIRS
  "."
  "../local-sdks/arduino-esp32/variants/feather_esp32"
)
register_component()

# in IDF 4 the approved method (with app_sources replacing COMPONENT_SRCS in
# the above GLOB) is idf_component_register, but this isn't present in 3.3:
#idf_component_register(
#  SRCS ${app_sources}
#  INCLUDE_DIRS "."
#    ../local-sdks/arduino-esp32/variants/feather_esp32
#)
