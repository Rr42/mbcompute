############################################################################
# File name: Makefile (./)
# Dev: GitHub@Rr42
# Code version: v1.1
# License:
#  Copyright 2022 Ramana R
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
# Description: 
# This code facilitates easy compilation of the MB core
#  interface and compute engine library.
############################################################################

# Decide whether the commands will be shown or not
VERBOSE = FALSE

# Subdirectories/libs to make
LIBS = mbcompute_lib mbcsupport_lib

# Subdirectories/apps to make
APPS = core

TARGETS = $(LIBS) $(APPS)

# Hide or not the calls depending of VERBOSE
ifeq ($(VERBOSE),TRUE)
	HIDE =  
else
	HIDE = @
endif

# Configuring based on target OS
ifeq ($(TARGETOS),LINUX)
# Linux (x64)
# Setting build path
	BUILDPTH := build/linux
# Setting complier
	# CXX := x86_64-linux-gnu-g++
	CXX := g++
	CXXOPTS := 
# Setting archiver
	# AR := x86_64-linux-gnu-ar
	AR := ar
	AROPTS := 
else ifeq ($(TARGETOS),WIN64)
# Windows (x64) (using MINGW64)
# Setting build path
	BUILDPTH := build/win64
# Setting complier
	CXX := x86_64-w64-mingw32-g++
	CXXOPTS := 
# Setting archiver
	AR := x86_64-w64-mingw32-ar
	AROPTS := 
else ifeq ($(TARGETOS),WIN32)
# Windows (x32) (using MINGW64)
# Setting build path
	BUILDPTH := build/win32
# Setting complier
	CXX := i686-w64-mingw32-g++
	CXXOPTS := 
# Setting archiver
	AR := i686-w64-mingw32-ar
	AROPTS := 
else
# Default
# Setting build path
	BUILDPTH := build
# Setting complier
	CXX := g++
	CXXOPTS := 
# Setting archiver
	AR := ar
	AROPTS := 
endif

TOPTARGETS := all clean

selftest:
	$(HIDE)echo '####################################'
	$(HIDE)echo '              Self test             '
	$(HIDE)echo '####################################'

config:
	$(HIDE)echo '####################################'
	$(HIDE)echo '           Configuration            '
	$(HIDE)echo '####################################'
	$(HIDE)echo Target platform: $(TARGETOS)
	$(HIDE)echo Build directory: $(BUILDPTH)
	$(HIDE)echo Using CXX: $(CXX)

$(TOPTARGETS): config $(TARGETS)
$(TARGETS):
	$(HIDE)echo '####################################'
	$(HIDE)echo Processing $@
	$(HIDE)echo '####################################'
	$(HIDE)$(MAKE) -C $@ VERBOSE=$(VERBOSE) BUILDPTH=$(BUILDPTH) CXX=$(CXX) CXXOPTS=$(CXXOPTS) AR=$(AR) AROPTS=$(AROPTS) $(MAKECMDGOALS)

.PHONY: config $(TOPTARGETS) $(TARGETS)
