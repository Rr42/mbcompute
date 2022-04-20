############################################################################
# File name: Makefile (./)
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

TOPTARGETS := all clean

$(TOPTARGETS): $(TARGETS)
$(TARGETS):
	$(eval export VERBOSE := $(VERBOSE))
	$(HIDE)echo '####################################'
	$(HIDE)echo Processing $@
	$(HIDE)echo '####################################'
	$(HIDE)$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(TARGETS)
