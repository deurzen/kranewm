PROJECT = kranewm

OBJDIR = obj
SRCDIR = src

BASE_SRC_FILES := src/main.cc src/kranewm.cc src/focus_group.cc src/util.cc src/workspace.cc src/ewmh.cc
BASE_OBJ_FILES := $(patsubst src/%.cc,obj/%.o,${BASE_SRC_FILES})

X_SRC_FILES := $(shell find $(SRCDIR) -name 'x_*.cc')
X_OBJ_FILES := $(patsubst src/%.cc,obj/%.o,${X_SRC_FILES})

CLIENT_SRC_FILES := $(shell find $(SRCDIR) -name 'client_*.cc')
CLIENT_OBJ_FILES := $(patsubst src/%.cc,obj/%.o,${CLIENT_SRC_FILES})

H_FILES = $(wildcard src/*.hh)
SRC_FILES := $(shell find $(SRCDIR) -name '*.cc')
OBJ_FILES := ${BASE_OBJ_FILES} ${X_OBJ_FILES} ${CLIENT_OBJ_FILES}
DEPS = $(OBJ_FILES:%.o=%.d)

RELEASE = release/$(PROJECT)
BIN = bin/$(PROJECT)
TARGET ?= $(RELEASE)

CXXFLAGS ?= -std=c++17
CXXFLAGS += `pkg-config --cflags x11`

LDFLAGS += `pkg-config --libs x11`

DEBUG_FLAGS = -Wall -g -DDEBUG

CC = g++
