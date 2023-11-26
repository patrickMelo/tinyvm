#
# Source/Makefile
#
# This file is part of the tinyVM source code.
# Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
#

# Common Variables

CXX				= clang++
CXX_FLAGS		= -O3 -std=c++0x -fno-rtti -fno-exceptions -Wno-sign-compare -Wno-format-security -Wno-narrowing -D_FILE_OFFSET_BITS=64 -pthread
DEBUG_FLAGS		= -g3 -DVM_DEBUG=1
INCLUDES		= -I$(CORE_DIRECTORY) -I$(SOURCE_DIRECTORY)/
LIBS			=
STRIP			= @true
STRIP_EXE		= strip

# Objects

CORE_OBJECTS	= $(patsubst %.cxx, %.o, $(shell find $(CORE_DIRECTORY) -iname "*.cxx" ) )
VM_OBJECTS		= $(patsubst %.cxx, %.o, $(shell find $(SOURCE_DIRECTORY) -iname "*.cxx" ) )

OBJECTS 		= $(CORE_OBJECTS) $(VM_OBJECTS)

# Linux Variables

LINUX_CXX_FLAGS	=
LINUX_LIBS		= -lm

# Windows Variables

WINDOWS_CXX_32		= i686-w64-mingw32-g++
WINDOWS_CXX_64		= x86_64-w64-mingw32-g++
WINDOWS_CXX_FLAGS	= -static-libgcc -static-libstdc++
WINDOWS_LIBS		=
WINDOWS_STRIP_32	= i686-w64-mingw32-strip
WINDOWS_STRIP_64	= x86_64-w64-mingw32-strip

# "Final" Variables

ifndef TARGET
	TARGET = linux
endif

ifndef TYPE
	TYPE = debug
endif

ifndef BITS
	BITS = 64
endif

ifeq ($(BITS), 32)
	ARCH		= i686
	CXX_FLAGS	+= -m32
else
	ARCH		= x86_64
	CXX_FLAGS	+= -m64
endif

ifeq ($(TARGET), linux)
	CXX_FLAGS	+= $(LINUX_CXX_FLAGS)
	LIBS		+= $(LINUX_LIBS)
endif

ifeq ($(TARGET), windows)
	ARCH		:= $(ARCH).exe
	CXX_FLAGS	+= $(WINDOWS_CXX_FLAGS)
	LIBS		+= $(WINDOWS_LIBS)

	ifeq ($(BITS), 32)
		CXX			= $(WINDOWS_CXX_32)
		STRIP_EXE	= $(WINDOWS_STRIP_32)
	else
		CXX			= $(WINDOWS_CXX_64)
		STRIP_EXE	= $(WINDOWS_STRIP_64)
	endif
endif

ifeq ($(TYPE), debug)
	CXX_FLAGS	+= $(DEBUG_FLAGS)
	ARCH		:= debug.$(ARCH)
endif

ifeq ($(TYPE), release)
	STRIP = $(STRIP_EXE) -x $(BINARY_PATH).$(ARCH)
endif

ifeq ($(BITS), 32)
	CXX_FLAGS += -m32
else
	CXX_FLAGS += -m64
endif

# Targets

%.o: %.cxx
	$(CXX) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

all: $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $(OBJECTS) $(LIBS) -o $(BINARY_PATH).$(ARCH)
	$(STRIP) $(BINARY_PATH).$(ARCH)

clean:
	rm -rf $(OBJECTS)

help:
	@echo ""
	@echo "Usage: make TARGET=<target name> TYPE=<debug|release> BITS=<32|64>"
	@echo ""
	@echo "Available targets:"
	@echo " - linux"
	@echo " - windows"
	@echo ""
	@echo "Defaults to: linux, debug, 64"
