PROJECT = kranewm

OBJDIR = obj
SRCDIR = src

BIN = bin
DESTDIR = /usr/local/bin

X_DATA_SRC_FILES := $(wildcard src/x-data/*.cc)
X_DATA_OBJ_FILES := $(patsubst src/x-data/%.cc,obj/%.o,${X_DATA_SRC_FILES})

BASE_SRC_FILES := $(wildcard src/*.cc)
BASE_OBJ_FILES := $(patsubst src/%.cc,obj/%.o,${BASE_SRC_FILES})

H_FILES := $(shell find $(SRCDIR) -name '*.hh')
SRC_FILES := $(shell find $(SRCDIR) -name '*.cc')
OBJ_FILES := ${X_DATA_OBJ_FILES} ${BASE_OBJ_FILES}
DEPS = $(OBJ_FILES:%.o=%.d)

SANFLAGS = -fsanitize=undefined -fsanitize=address -fsanitize-address-use-after-scope
CXXFLAGS = -std=c++17 -flto
LDFLAGS = `pkg-config --libs x11` -flto

DEBUG_CXXFLAGS = -Wall -g -DDEBUG ${SANFLAGS}
DEBUG_LDFLAGS = ${SANFLAGS}
RELEASE_CXXFLAGS = -O3

CC = g++
