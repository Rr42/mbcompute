############################################################################
# File name: Makefile (selftest/)
# Dev: GitHub@Rr42
# Code version: v1.0
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
#  interface.
############################################################################

# Set project directory one level above of Makefile directory. $(CURDIR) is a GNU make variable containing the path to the current working directory
PROJDIR := $(realpath $(CURDIR)/..)
SOURCEDIR := $(PROJDIR)
BUILDPTH := build
BUILDDIR := $(PROJDIR)/$(BUILDPTH)

# Name of the final executable
TARGET = $(BUILDDIR)/linux/mbconsole

# Tests to run
TESTS = ./test1.sh ./test2.sh ./test3.sh

# Decide whether the commands will be shown or not
VERBOSE = FALSE

SHELL = /bin/bash
GREP = /bin/grep

# Hide or not the calls depending of VERBOSE
ifeq ($(VERBOSE),TRUE)
	HIDE =  
else
	HIDE = @
endif

all: $(TESTS)
	$(HIDE)echo All tests passed!

$(TESTS):
	$(HIDE)echo Running $@
	$(HIDE)$(SHELL) $@ $(TARGET) | $(GREP) "PASS" > /dev/null || (echo "Test $@ failed" && exit 1)

.PHONY: all $(TESTS)
