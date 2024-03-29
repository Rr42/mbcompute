/****************************************************************************
* File name: mbcsupport_lib.hpp
* Version: v1.1
* Dev: GitHub@Rr42
* License:
*  Copyright 2023 Ramana R
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
* Description:
*  Library header for support functions and classes
*  for the MB compute engine.
****************************************************************************/
#ifndef __MB_COMPUTE_SUPPORT_LIB__

#define __MB_COMPUTE_SUPPORT_LIB__
/* Includes */
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

namespace mbcs{

/* Class declarations */

/* Parser class for processing CLI arguments */
class CLIParser{
/* Reference link: https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c */
private:
    std::vector<std::string> tokens;
public:
    /* Constructor for CLIParser class */
    CLIParser(int argc, char **argv);

    /* Destructor for CLIParser class */
    ~CLIParser(void);

    /* Method returns non empty string is a value is found for given argument */
    const std::string getCmdOption(const std::string option);

    /* Method returns true if option exists in the argument list */
    bool cmdOptionExists(const std::string option);

};

/* Common function headers */
template<typename element_type> std::string get_printable_vector(const std::vector<element_type>);

}

#endif
