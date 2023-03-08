/****************************************************************************
* File name: mbcsupport_lib.cpp
* Version: v1.1.1
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
*  Library containing support functions and classes
*  for the MB compute engine.
****************************************************************************/

#include "mbcsupport_lib.hpp"

namespace mbcs{

/* Class definitions */

/* Definitions for CLIParser class */
CLIParser::CLIParser (int argc, char **argv){
    for (int i=1; i < argc; ++i)
        this->tokens.push_back(std::string(argv[i]));
}

CLIParser::~CLIParser(void){
}

const std::string CLIParser::getCmdOption(const std::string option){
    /* Searchfor the substring option in the loaded tokens */
    std::vector<std::string>::const_iterator itr;
    itr = std::find_if(this->tokens.cbegin(), this->tokens.cend(), [option](const std::string& str){ return str.find(option) != std::string::npos; });
    if (itr != this->tokens.cend())
        return *itr;
    return "";
}

bool CLIParser::cmdOptionExists(const std::string option){
    return std::find_if(this->tokens.cbegin(), this->tokens.cend(), [option](const std::string& str){ return str.find(option) != std::string::npos; }) != this->tokens.cend();
}

/* Common function definitions */

/* Function converts a std::vector into a printable string */
// std::string get_printable_vector(std::vector<std::string> array){
template<typename element_type> std::string get_printable_vector(const std::vector<element_type> array){
    std::ostringstream printable;
    printable << "[" << array.size() << "]{ ";
    for (auto element : array)
        printable << element << " ";
    printable << "}";
    return printable.str();
}

/* Forward deceleration of supported/tested forms of get_printable_vector template */
template std::string get_printable_vector(const std::vector<std::string>);
template std::string get_printable_vector(const std::vector<double>);

}
