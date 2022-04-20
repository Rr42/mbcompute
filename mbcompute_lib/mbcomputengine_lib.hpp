/****************************************************************************
* File name: mbcomputengine_lib.hpp
* Version: v1.0
* Dev: GitHub@Rr42
* License:
*  Copyright 2022 Ramana R
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
*  Display library header for the 8x11 LED display.
****************************************************************************/
#ifndef __MB_COMPUTE_ENGINE_LIB__

#define __MB_COMPUTE_ENGINE_LIB__
/* Includes */
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <stack>

namespace mbc{

/* Class declarations */

/* Core compute engine class */
class Engine{
private:
    std::vector<std::string> _cmdBuffer;
    std::vector<std::string> _evalBuffer;
public:
    /* Constructor for Engine class */
    Engine();

    /* Distructor for Engine class */
    ~Engine(void);

    /* Function to load an expression line into the command buffer. */
    bool load(std::string);

    /* Function will try to evaluate the expression in the command buffer.
      If the evaluation was successfull the command buffer will be cleared and functionwill return true
      If the evaluation fails the function will return false. */
    bool eval(void);
};

/* Evaluator class for processing mathematical expressions */
class Evaluator{
private:
    std::vector<std::string> _expression_infix;
    std::vector<std::string> _expression_postfix;

    /* Function returns precedence of the operator given.
      The return value will be in the range [0, 3] */
    int getOPP(char);
public:
    /* Constructors for Evaluator class */
    Evaluator(const std::string);
    Evaluator(void);

    /* Distructor for Evaluator class */
    ~Evaluator(void);

    /* Function parses the given string expression into a workable list.
      The given string will be split into numbers (double in string form)
      and all other characters while taking SI prefixes into consideration.

      This function returns its object so operations can be cascaded. */
    Evaluator parseExpr(std::string);

    /* Function converts internal infix expression buffer to postfix.
      This function returns its object so operations can be cascaded. */
    Evaluator convertToPostfix(void);

    /* Function evaluates the given postfix expression. */
    double evaluatePostfix(void);

    /* Function returns the internal infix expression buffer. */
    const std::vector<std::string> getInfixBuffer(void);

    /* Function returns the internal postfix expression buffer. */
    const std::vector<std::string> getPostfixBuffer(void);
};

/* Common function headers */

}

#endif
