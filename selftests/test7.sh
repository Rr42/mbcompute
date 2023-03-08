#!/bin/bash
#############################################################################
# File name: test7.sh
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
#  Seventh self test for console application.
#  This test checks some advanced hierarchical function call and
#  definition capabilities.
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
printf "Running test: var2=1;var5=10;asc(bas(10*log(3**++5),var2))+2+lol(sdf(123, 4), fth(351+var5))\n"
result=`$mb_app $options --command="asc(var1 \"double\") : var1-1 \"Test 1\"\nbas(var1 \"double\", var2 \"double\") : var1*asc(var2) \"Test 2\"\nlol(var1 \"double\", var2 \"double\", _var3=15 \"double\") : var1-(var2*_var3) \"Test 3\"\nsdf(var1 \"double\", var2 \"double\") : var1+bas(var2,-4) \"Test 4\"\nfth(var1 \"double\", var21=1 \"double\", var22=-3 \"double\") : var1+1+(var21*var22) \"Test 5\"\nvar2=1;var5=10;asc(bas(10*log(3**++5),var2))+2+lol(sdf(123, 4), fth(351+var5))\nexit" | tail -n 1`
printf "Result: $result"
if [ "$result" == "-5281" ]; then
    printf " - PASS\n"
else
    printf " - FAIL\n"
fi

printf "Cleaning up...\n"
pkill -SIGKILL mbconsole
exit
