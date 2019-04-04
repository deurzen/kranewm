PROJECT = kranewm

OBJDIR = obj
SRCDIR = src

X_WRAPPER_SRC_FILES := $(wildcard src/x-wrapper/*.cc)
X_WRAPPER_OBJ_FILES := $(patsubst src/x-wrapper/%.cc,obj/%.o,${X_WRAPPER_SRC_FILES})

BASE_SRC_FILES := $(wildcard src/*.cc)
BASE_OBJ_FILES := $(patsubst src/%.cc,obj/%.o,${BASE_SRC_FILES})

H_FILES := $(shell find $(SRCDIR) -name '*.hh')
SRC_FILES := $(shell find $(SRCDIR) -name '*.cc')
OBJ_FILES := ${X_WRAPPER_OBJ_FILES} ${BASE_OBJ_FILES}
DEPS = $(OBJ_FILES:%.o=%.d)

RELEASE = release/$(PROJECT)
BIN = bin/$(PROJECT)
TARGET ?= $(RELEASE)

CXXFLAGS ?= -std=c++17
CXXFLAGS += `pkg-config --cflags x11`

LDFLAGS += `pkg-config --libs x11`

DEBUG_FLAGS = -Wall -g -DDEBUG

CC = g++
