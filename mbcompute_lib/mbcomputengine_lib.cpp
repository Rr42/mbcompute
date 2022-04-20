/****************************************************************************
* File name: mbcomputengine_lib.cpp
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
*  Display library implimentation for the 8x11 LED display.
****************************************************************************/

#include "mbcomputengine_lib.hpp"

namespace mbc{

/* Class definitions */

/* Engine class definitions */
Engine::Engine(){
}

Engine::~Engine(void){
}

bool Engine::load(std::string line){
    /* Remove spaces from line */
    line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char x) { return std::isspace(x); }), line.end());

    /* Add line to command stack */
    this->_cmdBuffer.push_back(line);

    return true;
}

bool Engine::eval(void){
    /* Clear command buffer */
    this->_cmdBuffer.clear();

    return true;
}

/* Evaluator class definitions */
Evaluator::Evaluator(const std::string expression){
    this->parseExpr(expression);
}

Evaluator::Evaluator(void){
}

Evaluator::~Evaluator(void){
}

const std::vector<std::string> Evaluator::getInfixBuffer(void){
    return this->_expression_infix;
}

const std::vector<std::string> Evaluator::getPostfixBuffer(void){
    return this->_expression_postfix;
}

int Evaluator::getOPP(char opr){
    switch (opr){
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        case '^':
            return 3;
        default:
            return 0;
    }
}

/* SI prefixes
+-------+--------+------------+-----------------------------+-----------------------+
| Name  | Symbol | Scientific |          Decimal            | English (Short scale) |
+-------+--------+------------+-----------------------------+-----------------------+
| yotta |  Y     | 1E+24      |  1000000000000000000000000  |  septillion           |
| zetta |  Z     | 1E+21      |  1000000000000000000000     |  sextillion           |
| exa   |  E     | 1E+18      |  1000000000000000000        |  quintillion          |
| peta  |  P     | 1E+15      |  1000000000000000           |  quadrillion          |
| tera  |  T     | 1E+12      |  1000000000000              |  trillion             |
| giga  |  G     | 1E+9       |  1000000000                 |  billion              |
| mega  |  M     | 1E+6       |  1000000                    |  million              |
| kilo  |  k     | 1E+3       |  1000                       |  thousand             |
| hecto |  h     | 1E+2       |  100                        |  hundred              |
| deca  |  da    | 1E+1       |  10                         |  ten                  |
|       |        | 1E+0       |  1                          |  one                  |
| deci  |  d     | 1E-1       |  0.1                        |  tenth                |
| centi |  c     | 1E-2       |  0.01                       |  hundredth            |
| milli |  m     | 1E-3       |  0.001                      |  thousandth           |
| micro |  μ/u   | 1E-6       |  0.000001                   |  millionth            |
| nano  |  n     | 1E-9       |  0.000000001                |  billionth            |
| pico  |  p     | 1E-12      |  0.000000000001             |  trillionth           |
| femto |  f     | 1E-15      |  0.000000000000001          |  quadrillionth        |
| atto  |  a     | 1E-18      |  0.000000000000000001       |  quintillionth        |
| zepto |  z     | 1E-21      |  0.000000000000000000001    |  sextillionth         |
| yocto |  y     | 1E-24      |  0.000000000000000000000001 |  septillionth         |
+-------+--------+------------+-----------------------------+-----------------------+
*/
Evaluator Evaluator::parseExpr(std::string expr){
    /* Remove spaces in input before proceeding */
    expr.erase(std::remove_if(expr.begin(), expr.end(), [](unsigned char x) { return std::isspace(x); }), expr.end());

    /* flag to mark if a number is being parsed */
    bool flag_number = false;

    /* Iterate input expression */
    for (std::string::iterator it = expr.begin(); it != expr.end(); ++it){
        if (not std::isdigit(*it))
            if (flag_number){
                /* Convert SI prefixes to their scientific from */
                switch (*it){
                    case 'Y':
                        this->_expression_infix.back() += "E+24";
                        break;
                    case 'Z':
                        this->_expression_infix.back() += "E+21";
                        break;
                    case 'E':
                        this->_expression_infix.back() += "E+28";
                        break;
                    case 'P':
                        this->_expression_infix.back() += "E+15";
                        break;
                    case 'T':
                        this->_expression_infix.back() += "E+12";
                        break;
                    case 'G':
                        this->_expression_infix.back() += "E+9";
                        break;
                    case 'M':
                        this->_expression_infix.back() += "E+6";
                        break;
                    case 'k':
                        this->_expression_infix.back() += "E+3";
                        break;
                    case 'h':
                        this->_expression_infix.back() += "E+2";
                        break;
                    // case 'da':
                    //     this->_expression_infix.back() += "E+1";
                    //     break;
                    case 'd':
                        if (*(it+1) == 'a'){
                            /* Case for da */
                            this->_expression_infix.back() += "E+1";
                            /* Skip next character */
                            ++it;
                        }
                        else
                            /* Case for d */
                            this->_expression_infix.back() += "E-1";
                        break;
                    case 'c':
                        this->_expression_infix.back() += "E-2";
                        break;
                    case 'm':
                        this->_expression_infix.back() += "E-3";
                        break;
                    case 'u':
                        this->_expression_infix.back() += "E-6";
                        break;
                    case 'n':
                        this->_expression_infix.back() += "E-9";
                        break;
                    case 'p':
                        this->_expression_infix.back() += "E-12";
                        break;
                    case 'f':
                        this->_expression_infix.back() += "E-15";
                        break;
                    case 'a':
                        this->_expression_infix.back() += "E-18";
                        break;
                    case 'z':
                        this->_expression_infix.back() += "E-21";
                        break;
                    case 'y':
                        this->_expression_infix.back() += "E-24";
                        break;
                    case '.':
                        this->_expression_infix.back() += *it;
                        break;
                    default:
                        /* ![ALERT]! simething is wrong! */
                        this->_expression_infix.push_back(std::string(1, *it));
                        /* Mark the end of a number */
                        flag_number = false;
                        break;
                }
            } else{
                /* Append all other characters as is */
                this->_expression_infix.push_back(std::string(1, *it));
                flag_number = false;
            }
        else{
            if (flag_number)
                /* Accumulate the characters of the same number */
                this->_expression_infix.back() += *it;
            else
                /* Append to list if this is the first character of the number */
                this->_expression_infix.push_back(std::string(1, *it));
            flag_number = true;
        }
    }

    /* Return back object so functions can be cascaded */
    return *this;
}

/*  Infix to postfix algorithm
 * Reference link: https://iq.opengenus.org/infix-to-postfix-expression-stack/#:~:text=To%20convert%20Infix%20expression%20to,maintaining%20the%20precedence%20of%20them.
 * Step 1 : Scan the Infix Expression from left to right.
 * Step 2 : If the scanned character is an operand, append it with final Infix to Postfix string.
 * Step 3 : Else,
 *  Step 3.1 : If the precedence order of the scanned(incoming) operator is greater than the precedence order of the operator in the stack (or the stack is empty or the stack contains a ‘(‘ or ‘[‘ or ‘{‘), push it on stack.
 *  Step 3.2 : Else, Pop all the operators from the stack which are greater than or equal to in precedence than that of the scanned operator.
 *             After doing that Push the scanned operator to the stack.
 *             (If you encounter parenthesis while popping then stop there and push the scanned operator in the stack.)
 * Step 4 : If the scanned character is an ‘(‘ or ‘[‘ or ‘{‘, push it to the stack.
 * Step 5 : If the scanned character is an ‘)’or ‘]’ or ‘}’, pop the stack and and output it until a ‘(‘ or ‘[‘ or ‘{‘ respectively is encountered, and discard both the parenthesis.
 * Step 6 : Repeat steps 2-6 until infix expression is scanned.
 * Step 7 : Print the output.
 * Step 8 : Pop and output from the stack until it is not empty.
 */
Evaluator Evaluator::convertToPostfix(void){
    std::stack<std::string> stack;

    for (auto it = this->_expression_infix.begin(); it != this->_expression_infix.end(); ++it){
        /* If scanned character is open bracket push it on stack */
        if(*it == "(" || *it == "[" || *it == "{")
            stack.push(*it);
        /* If scanned character is opened bracket pop all literals from stack till matching open bracket gets popped */
        else if(*it == ")" || *it == "]" || *it == "}"){
            if(*it == ")"){
                while(stack.top() != "("){
                    this->_expression_postfix.push_back(stack.top());
                    stack.pop();
                }
            }
            if(*it == "]"){
                while(stack.top() != "["){
                    this->_expression_postfix.push_back(stack.top());
                    stack.pop();
                }
            }
            if(*it == "}"){
                while(stack.top() != "{"){
                    this->_expression_postfix.push_back(stack.top());
                    stack.pop();
                }
            }
            stack.pop();
        } else if(*it == "+" || *it == "-" || *it == "*" || *it == "/" || *it == "^"){
            /* If scanned character is operator */
            /* very first operator of expression is to be pushed on stack */
            if(stack.empty()){
                stack.push(*it);

            } else{
                /* Check the precedence order of instack(means the one on top of stack) and incoming operator,
                 * if instack operator has higher priority than incoming operator pop it out of stack&put it in
                 * final postfix expression, on other side if precedence order of instack operator is less than i
                 * coming operator, push incoming operator on stack.
                 */
                if(getOPP(stack.top().c_str()[0]) >= getOPP((*it).c_str()[0])){
                    this->_expression_postfix.push_back(stack.top());
                    stack.pop();
                    stack.push(*it);
                } else{
                    stack.push(*it);
                }
            }
        } else{
            /* If literal is operand, put it on to final postfix expression */
            this->_expression_postfix.push_back(*it);
        }
    }

    /* Poping out all remaining operator literals & adding to final postfix expression */
    if(!stack.empty()){
        while(!stack.empty()){
            this->_expression_postfix.push_back(stack.top());
            stack.pop();
        }
    }

    /* Return back object so functions can be cascaded */
    return *this;
}

/* Algorithm to evaluate a postfix expression
 * Reference link: https://www.geeksforgeeks.org/stack-set-4-evaluation-postfix-expression/
 * 1) Create a stack to store operands (or values).
 * 2) Scan the given expression and do the following for every scanned element.
 *   a) If the element is a number, push it into the stack.
 *   b) If the element is an operator, pop operands for the operator from the stack.
 *      Evaluate the operator and push the result back to the stack.
 * 3) When the expression is ended, the number in the stack is the final answer
 */
double Evaluator::evaluatePostfix(void){
    std::stack<double> stack;
 
    /* Scan all elements one by one */
    for (auto it = this->_expression_postfix.begin(); it != this->_expression_postfix.end(); ++it){
        /* If the scanned element is an operand (number here),
        push it to the stack. */
        if (isdigit((*it).c_str()[0]))
            stack.push(std::atof((*it).c_str()));
        /* If the scanned element is an operator, pop two
        elements from stack apply the operator */
        else{
            double val1 = stack.top();
            stack.pop();
            double val2 = stack.top();
            stack.pop();

            switch ((*it).c_str()[0]){
                case '+':
                    stack.push(val2 + val1);
                    break;
                case '-':
                    stack.push(val2 - val1);
                    break;
                case '*':
                    stack.push(val2 * val1);
                    break;
                case '/':
                    stack.push(val2/val1);
                    break;
            }
        }
    }
    return stack.top();
}

/* Common function definitions */

}
