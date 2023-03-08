#!/bin/bash
#############################################################################
# File name: test4.sh
# Version: v1.0
# Dev: GitHub@Rr42
# License:
#  Copyright 2023 Ramana R
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
#  Fourth self test for console application.
#  This test checks some of the more advanced compute capabilities.
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

# Run test commands and get the last line of the output for comparison
printf "Running test: ++1**++(10%%5--8)\n"
result=`$mb_app $options --command="++1**++(10%--8)\nexit" | tail -n 1`
printf "Result: $result"
if [ "$result" == "16" ]; then
    printf " - PASS\n"
else
    printf " - FAIL\n"
fi

printf "Cleaning up...\n"
pkill -SIGKILL mbconsole
exit
