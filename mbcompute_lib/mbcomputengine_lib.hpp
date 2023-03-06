/****************************************************************************
* File name: mbcomputengine_lib.hpp
* Version: v1.2
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
*  The MB compute engine library header containing declarations for
*  expression parsing and evaluation classes.
****************************************************************************/
#ifndef __MB_COMPUTE_ENGINE_LIB__

#define __MB_COMPUTE_ENGINE_LIB__
/* Includes */
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <stack>
#include <sstream>
#include <cassert>
#include <regex>
#include <cmath>

#include <iostream>

namespace mbc{

/* Operator help structure */
struct OperatorHelp
{
    /* Operator precedence (lower value = higher precedence) */
    unsigned int order;
    /* Operator symbol */
    std::string oop;
    /* Operator category */
    std::string cat;
    /* Operator description */
    std::string desc;
};

/* List of supported operators */
const std::vector<OperatorHelp> SUPPORTED_OOPS{
    {1,  "++", "Arithmetic", "Increment"},
    {1,  "--", "Arithmetic", "Decrement"},
    {2,  "**", "Arithmetic", "Power"},
    {3,  "*",  "Arithmetic", "Multiply"},
    {3,  "/",  "Arithmetic", "Divide"},
    {3,  "%",  "Arithmetic", "Reminder"},
    {4,  "+",  "Arithmetic", "Add"},
    {4,  "-",  "Arithmetic", "Subtract"},
    {5,  "<<", "Bitwise",    "Left shift"},
    {5,  ">>", "Bitwise",    "Right shift"},
    {6,  "<",  "Logical",    "Less than"},
    {6,  ">",  "Logical",    "Grater than"},
    {7,  "==", "Logical",    "Equals"},
    {8,  "!=", "Logical",    "Not equals"},
    {9,  "&",  "Bitwise",    "AND"},
    {10,  "^",  "Bitwise",    "XOR"},
    {11, "|",  "Bitwise",    "OR"},
    {12, "!",  "Logical",    "NOT"},
    {13, "&&", "Logical",    "AND"},
    {14, "^^", "Logical",    "XOR"},
    {15, "||", "Logical",    "OR"},
};

/* Class declarations */

/* Evaluator class for processing mathematical expressions */
class Evaluator{
private:
    std::vector<std::string> _expression_infix;
    std::vector<std::string> _expression_postfix;

    unsigned int max_precedence;
    /* Method returns precedence of the operator given.
      The return value will be in the range [0, 3] */
    int getOPP(std::string);
public:
    /* Constructors for Evaluator class */
    Evaluator(const std::string);
    Evaluator(void);

    /* Destructor for Evaluator class */
    ~Evaluator(void);

    /* Method parses the given string expression into a workable list.
      The given string will be split into numbers (double in string form)
      and all other characters while taking SI prefixes into consideration.

      This method returns its object so operations can be cascaded. */
    Evaluator parseExpr(std::string);

    /* Method converts internal infix expression buffer to postfix.
      This method returns its object so operations can be cascaded. */
    Evaluator convertToPostfix(void);

    /* Method evaluates the given postfix expression.
      Returns 0 if no result was generated. */
    double evaluatePostfix(void);

    /* Method clears all buffers. */
    void clear(void);

    /* Method returns the internal infix expression buffer. */
    const std::vector<std::string> getInfixBuffer(void);

    /* Method returns the internal postfix expression buffer. */
    const std::vector<std::string> getPostfixBuffer(void);
};

/* Core compute engine class */
class Engine{
private:
    /* Executor object */
    mbc::Evaluator _runner;

    /* Command queue */
    std::vector<std::string> _cmdBuffer;

    /* Result queue and wiper */
    std::vector<std::string> _evalBuffer;
    std::size_t _evalWiper;

    /* Method returns true if given variable name is valid */
    bool checkVarName(std::string);
public:
    /* Expression variables */
    std::vector<std::string> _varNames;
    std::vector<double> _varValues;
    /* Constructor for Engine class */
    Engine();

    /* Destructor for Engine class */
    ~Engine(void);

    /* Method to load an expression line into the command buffer.
      This method returns its object so operations can be cascaded. */
    Engine load(std::string);

    /* Method will try to evaluate all the expression(s) loaded into the command buffer.
      If the evaluation was successful the command buffer will be cleared.
      This method returns its object so operations can be cascaded. */
    Engine eval(void);

    /* Method returns the oldest result that hasn't been returned form the results queue
      If there are no further results the method will return "null".

      For example if the return queue contains the following results:
        [1, 0.9, 100, 10e-19]
      Calling the gerResult method in succession will return:
        getResult() -> "1"
        getResult() -> "0.9"
        getResult() -> "100"
        getResult() -> "10e-19"
        getResult() -> ""
    */
    std::string getResult(void);
};

}

#endif
