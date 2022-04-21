#!/bin/bash
#############################################################################
# File name: test1.sh
# Version: v1.0
# Dev: GitHub@Rr42
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
#  First set of self tests for console application.
#############################################################################

# Check if the console application name+path was passed
if [[ -z $1 ]];
then 
    printf "Application name not provided!\n"
    exit
else
    mb_app=$1
    printf "Testing $mb_app"
if

mb_testout_pipe=/tmp/mbctestout.fifo
mb_testin_pipe=/tmp/mbctestin.fifo

printf "Creating test pipe\n"
mkfifo $mb_testout_pipe
mkfifo $mb_testin_pipe

$mb_app <$mb_testout_pipe >$mb_testin_pipe &

print "Terminating console application\n"
pkill -SIGKILL mbconsole

printf "Closing test pipe\n"
rm $mb_testout_pipe
rm $mb_testin_pipe