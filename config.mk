PROJECT = kranewm
BAR = kranebar
CLIENT = kranec

DEPENDENCIES = x11 xrandr xres libprocps spdlog

OBJDIR = obj
SRCDIR = src

BINDIR = bin
INSTALLDIR = /usr/local/bin

BAR_SRC_FILES := $(wildcard src/bar/*.cc)
BAR_OBJ_FILES := $(patsubst src/bar/%.cc,obj/bar/%.o,${BAR_SRC_FILES})

CLIENT_SRC_FILES := $(wildcard src/client/*.cc)
CLIENT_OBJ_FILES := $(patsubst src/client/%.cc,obj/client/%.o,${CLIENT_SRC_FILES})

CORE_SRC_FILES := $(wildcard src/core/*.cc)
CORE_OBJ_FILES := $(patsubst src/core/%.cc,obj/core/%.o,${CORE_SRC_FILES})

WINSYS_SRC_FILES := $(wildcard src/winsys/*.cc)
WINSYS_OBJ_FILES := $(patsubst src/winsys/%.cc,obj/winsys/%.o,${WINSYS_SRC_FILES})

X_DATA_SRC_FILES := $(wildcard src/winsys/xdata/*.cc)
X_DATA_OBJ_FILES := $(patsubst src/winsys/xdata/%.cc,obj/winsys/xdata/%.o,${X_DATA_SRC_FILES})

WINSYS_LINK_FILES := ${WINSYS_OBJ_FILES} ${X_DATA_OBJ_FILES}
BAR_LINK_FILES := ${WINSYS_OBJ_FILES} ${X_DATA_OBJ_FILES} ${BAR_OBJ_FILES}
CLIENT_LINK_FILES := ${WINSYS_OBJ_FILES} ${X_DATA_OBJ_FILES} ${CLIENT_OBJ_FILES}
CORE_LINK_FILES := ${WINSYS_OBJ_FILES} ${X_DATA_OBJ_FILES} ${CORE_OBJ_FILES}

H_FILES := $(shell find $(SRCDIR) -name '*.hh')
SRC_FILES := $(shell find $(SRCDIR) -name '*.cc')
OBJ_FILES := ${WINSYS_OBJ_FILES} ${X_DATA_OBJ_FILES} ${CORE_OBJ_FILES}
DEPS = $(OBJ_FILES:%.o=%.d)

SANFLAGS = -fsanitize=undefined -fsanitize=address -fsanitize-address-use-after-scope
CXXFLAGS = -std=c++20 `pkg-config --cflags ${DEPENDENCIES}`
LDFLAGS = `pkg-config --libs ${DEPENDENCIES}` -pthread

DEBUG_CXXFLAGS = -Wall -Wpedantic -Wextra -Wold-style-cast -g -DDEBUG ${SANFLAGS}
DEBUG_LDFLAGS = ${SANFLAGS}
RELEASE_CXXFLAGS = -march=native -mtune=native -O3 -flto
RELEASE_LDFLAGS = -flto

CC = g++
