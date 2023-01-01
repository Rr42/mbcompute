#!/bin/bash
#############################################################################
# File name: test2.sh
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
#  Second self test for console application.
#############################################################################

# Check if the console application name+path was passed
if [[ -z $1 ]]; then
    printf "Application name not provided!\n"
    exit
else
    mb_app=$1
    printf "Testing $mb_app\n"
fi

# Execution options
options="--silent"

# Run test commands and get the last line of the output fro comparison
printf "Running test: a1=b=d=10*3.1415*10\n"
result=`$mb_app $options --command="a1=b=d=10*3.1415*10\nexit" | tail -n 1`
printf "Result: $result"
if [ "$result" == "314.15" ]; then
    printf " - PASS\n"
else
    printf " - FAIL\n"
fi

printf "Cleaning up...\n"
pkill -SIGKILL mbconsole
exit
