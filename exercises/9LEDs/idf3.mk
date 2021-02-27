# ESP IDF version 3 project Makefile for 9LEDs

# basic config: CHANGE THESE for new projects
PROJECT_NAME := 9LEDs
PROJECT_VER := 1.0.0

# TODO the library additions are messy... note also each library source dir
# needs an empty component.mk

# tone down compile warnings; make sure we see the correct pin defs header
CXXFLAGS += \
-I$(IDF_PATH)/../arduino-esp32/variants/feather_esp32 \
-I$(CURDIR)/lib/Adafruit_Motor_Shield_V2_Library \
-I$(CURDIR)/lib/Adafruit_Motor_Shield_V2_Library/utility \
-Wno-error=return-type -Wno-write-strings \
-Wno-return-type -Wno-pointer-arith -Wno-cpp -Wno-unused-variable \
-Wno-int-to-pointer-cast -Wno-missing-field-initializers -fpermissive

# Arduino core (and lib/)
EXTRA_COMPONENT_DIRS += \
    $(PROJECT_PATH)/local-sdks/arduino-esp32 \
    $(PROJECT_PATH)/lib/Adafruit_Motor_Shield_V2_Library \
    $(PROJECT_PATH)/lib/Adafruit_Motor_Shield_V2_Library/utility
COMPONENT_SRC_DIRS += \
    $(PROJECT_PATH)/lib/Adafruit_Motor_Shield_V2_Library \
    $(PROJECT_PATH)/lib/Adafruit_Motor_Shield_V2_Library/utility

# IDF targets and settings
include $(IDF_PATH)/make/project.mk
