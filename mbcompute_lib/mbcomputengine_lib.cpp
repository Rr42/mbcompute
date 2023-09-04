/****************************************************************************
* File name: mbcomputengine_lib.cpp
* Version: v1.5
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
*  The MB compute engine library containing implementations for
*  expression parsing and evaluation.
****************************************************************************/

#include "mbcomputengine_lib.hpp"

namespace mbc{

/* Class definitions */

/* Engine class definitions */
Engine::Engine(){
    /* Initialise class members */
    this->_evalWiper = 0;
    this->_error_message.clear();
    this->_warning_message.clear();

    /* Define all default supported functions */
    this->_supported_functions = SUPPORTED_FUNS;
}

Engine::~Engine(void){
}

bool Engine::checkVarName(std::string name){
    /* If the first character is not an alphabet or _ it is not a valid variable name */
    if (!(isalpha(name[0]) || name[0] == '_'))
        return false;
    /* Loop through each character of the name and check for invalid characters */
    for (char chr : name)
        if (!(isalnum(chr) || chr == '_'))
            return false;
    return true;
}

const std::string Engine::replaceVars(const std::string cmd){
    std::string cmd_mod = "";
    this->_runner.parseExpr(cmd);
    for (std::string element : this->_runner.getInfixBuffer()){
        /* Check if this element exists in the list of known variable names */
        auto name_it = std::find(this->_varNames.cbegin(), this->_varNames.cend(), element);
        /* Make sure that this is a variable and not a function name (in which case the next element will be an opening parentheses)*/
        if (name_it != this->_varNames.cend() && !(std::next(name_it) != this->_varNames.cend() && *(name_it+1) == "("))
            cmd_mod += "("+std::to_string(this->_varValues[name_it-this->_varNames.cbegin()])+")";
        else
            cmd_mod += element;
    }
    this->_runner.clear();
    return cmd_mod;
}

const std::string Engine::evalFunctions(const std::string cmd){
    std::smatch match;
    std::regex filter(R"(([a-zA-Z0-9_]+)(\(.*\)+))");

    /* Check if an error occurred before (during one of the recursive calls) */
    if (!this->_error_message.empty())
        return std::string();

    /* Check if a function call exists in the cmd */
    if (!std::regex_search(cmd.cbegin(), cmd.cend(), match, filter))
        /* No function found return incoming command */
        return cmd;

    unsigned int open_count = 0;
    unsigned int close_count = 0;
    std::string buffer;
    bool flag_ch_added = false;
    std::string flattened_cmd;
    for (char ch : cmd){
        /* Reset flag */
        flag_ch_added = false;
        /* Count parentheses */
        if (ch == '('){
            ++open_count;
            buffer += ch;
            flag_ch_added = true;
        } else if (ch == ')'){
            ++close_count;
            buffer += ch;
            flag_ch_added = true;
        }
        /* Accumulate the function calls */
        if (open_count != 0){
            /* If the open and close counts match a function call has been detected */
            if (open_count == close_count){
                /* Evaluate the function call */
                std::string result;
                /* Check if this is a function call */
                if (!std::regex_match(buffer, match, filter)){
                    /* Clear buffers and continue */
                    open_count=0;
                    close_count=0;
                    continue;
                }
                assert(match.size() == 3);
                std::string match_fname = match[1].str();
                std::string match_args = match[2].str();
                /* Check if there are any other function calls inside */
                if (open_count > 1 && std::regex_search(match_args, filter)){
                    /* If more function calls are found, remove the outer call and recurse */
                    /* Recurse after removing the enclosing parentheses */
                    result = this->evalFunctions(match_args.substr(1, match_args.size() - 2));
                    /* Check if any errors occurred during the recursion */
                    if (!this->_error_message.empty())
                        return std::string();
                    /* Update buffer with the expanded inner functions */
                    buffer = std::regex_replace(buffer, std::regex(getRegExEscaped(match_args)), "("+result+")");
                    /* Update the argument match string */
                    std::regex_match(buffer, match, filter);
                    match_fname = match[1].str();
                    match_args = match[2].str().substr(1, match[2].str().size() - 2);
                }
                /* Expand arguments */
                size_t split_next = 0;
                size_t split_last = 0;
                std::vector<std::string> args;
                while ((split_next = match_args.find(",", split_last)) != std::string::npos) {
                    args.push_back(match_args.substr(split_last, split_next-split_last));
                    split_last = split_next+1;
                }
                if (args.empty())
                    args.push_back(match_args);
                else
                    args.push_back(match_args.substr(split_last));
                /* Remove extra brackets from the first and last arguments */
                if (args[0][0] == '(' && std::count(args[0].cbegin(), args[0].cend(), '(') != std::count(args[0].cbegin(), args[0].cend(), ')'))
                    args[0].erase(0, 1);
                if (args[args.size()-1].back() == ')' && std::count(args[args.size()-1].cbegin(), args[args.size()-1].cend(), '(') != std::count(args[args.size()-1].cbegin(), args[args.size()-1].cend(), ')'))
                    args[args.size()-1].erase(args[args.size()-1].size()-1, 1);

                /* Find the function in the supported list */
                auto fun_it = std::find_if(this->_supported_functions.cbegin(), this->_supported_functions.cend(), [match_fname](MetaFunction item){return item.name == match_fname;});
                if (fun_it == this->_supported_functions.cend()){
                    /* If function is not found set the error string and return */
                    this->_error_message += "[Engine] ERROR: Undefined function `"+match_fname+"` called!\n";
                    return std::string();
                }
                /* Check the argument list */
                if (args.size() > (*fun_it).arg_names.size()){
                    /* If function call has more arguments than its definition set the error string and return */
                    this->_error_message += "[Engine] ERROR: Too many arguments passed to function `"+match_fname+"`! `"+match_fname+"` got "+std::to_string(args.size())+" argument(s) but it's definition only takes "+std::to_string((*fun_it).arg_names.size())+" argument(s)\n";
                    return std::string();
                }
                /* Count the number of optional arguments in the function definition */
                size_t optional_count = 0;
                for (std::string item : (*fun_it).arg_names)
                    if (item.find("=") != std::string::npos)
                        ++optional_count;
                /* Check if the number of arguments passed is enough to cover all standard arguments */
                if (args.size() < (*fun_it).arg_names.size()-optional_count){
                    /* If function call does not have enough arguments to account for the number of standard arguments set the error string and return */
                    if (optional_count == 1)
                        this->_error_message += "[Engine] ERROR: Insufficent number of arguments passed to function `"+match_fname+"`! `"+match_fname+"` got "+std::to_string(args.size())+" argument(s) but it's definition requires "+std::to_string((*fun_it).arg_names.size())+" argument(s) out of which "+std::to_string(optional_count)+" is optional\n";
                    else
                        this->_error_message += "[Engine] ERROR: Insufficent number of arguments passed to function `"+match_fname+"`! `"+match_fname+"` got "+std::to_string(args.size())+" argument(s) but it's definition requires "+std::to_string((*fun_it).arg_names.size())+" argument(s) out of which "+std::to_string(optional_count)+" are optional\n";
                    return std::string();
                }

                /* Reconstruct function expression by replacing arguments */
                result = (*fun_it).expr;
                std::string arg_name = "";
                std::string arg_default = "";
                for (auto arg_it = (*fun_it).arg_names.cbegin(); arg_it != (*fun_it).arg_names.cend(); ++arg_it){
                    size_t arg_index = arg_it-(*fun_it).arg_names.cbegin();
                    /* If this argument has a default value separate the name and value */
                    if (std::regex_match(*arg_it, match, std::regex(R"(([a-zA-z0-9_)]+)=(.*))"))){
                        assert(match.size() == 3);
                        arg_name = match[1].str();
                        arg_default = match[2].str();
                    } else{
                        arg_name = *arg_it;
                        arg_default.clear();
                    }
                    if (arg_index < args.size())
                        /* If a respective argument has been passed use it */
                        result = std::regex_replace(result, std::regex(getRegExEscaped(arg_name)), "("+args[arg_index]+")");
                    else
                        /* If a respective argument has not been passed use the default */
                        result = std::regex_replace(result, std::regex(getRegExEscaped(arg_name)), "("+arg_default+")");
                }

                /* Evaluate function calls in the function expression (if any) */
                if (std::regex_search(result, match, filter)){
                    /* Check for internal reserved functions */
                    if (std::regex_match(result, std::regex(R"((__[a-zA-Z0-9_]+__)(\(.*\)+))"))){
                        /* If this is a reserved internal function directly evaluate the function */
                        /* Convert all the arguments into numbers */
                        std::vector<double> converted_args;
                        for (std::string arg : args){
                            /* Parse argument and concert for computation */
                            this->_runner.parseExpr(arg);
                            this->_runner.convertToPostfix();
                            /* Add evaluated argument to converted list */
                            converted_args.push_back(this->_runner.evaluatePostfix());
                            /* Clean up the runner */
                            this->_runner.clear();
                        }
                        /* Check which reserved internal function is being called */
                        if (match[1].str() == "__log__")
                            result = "("+std::to_string(std::log(converted_args[0]))+")";
                        else if (match[1].str() == "__log10__")
                            result = "("+std::to_string(std::log10(converted_args[0]))+")";
                        else if (match[1].str() == "_ceil__")
                            result = "("+std::to_string(std::ceil(converted_args[0]))+")";
                        else if (match[1].str() == "__floor__")
                            result = "("+std::to_string(std::floor(converted_args[0]))+")";
                        else if (match[1].str() == "__abs__")
                            result = "("+std::to_string(std::abs(converted_args[0]))+")";
                        else if (match[1].str() == "__cos__")
                            result = "("+std::to_string(std::cos(converted_args[0]))+")";
                        else if (match[1].str() == "__sin__")
                            result = "("+std::to_string(std::sin(converted_args[0]))+")";
                        else if (match[1].str() == "__tan__")
                            result = "("+std::to_string(std::tan(converted_args[0]))+")";
                        else if (match[1].str() == "__cosh__")
                            result = "("+std::to_string(std::cosh(converted_args[0]))+")";
                        else if (match[1].str() == "__sinh__")
                            result = "("+std::to_string(std::sinh(converted_args[0]))+")";
                        else if (match[1].str() == "__tanh__")
                            result = "("+std::to_string(std::tanh(converted_args[0]))+")";
                        else if (match[1].str() == "__pow__")
                            result = "("+std::to_string(std::pow(converted_args[0], converted_args[1]))+")";
                        else{
                            /* An unsupported internal function was detected, set the error string and return */
                            this->_error_message += "[Engine] ERROR: An unknown function call was detected `"+match[1].str()+"`!\n";
                            return std::string();
                        }
                    }
                    else{
                        /* If this is a standard function evaluate it and store it's result */
                        result = this->evalFunctions(result);
                        /* Check if any errors occurred during the recursion */
                        if (!this->_error_message.empty())
                            return std::string();
                    }
                }

                /* Replace function with the evaluated call in the flattened command */
                flattened_cmd += "("+result+")";

                /* Clear buffers and continue */
                buffer.clear();
                open_count=0;
                close_count=0;
            } else if (!flag_ch_added)
                /* Otherwise append the character to the buffer */
                buffer += ch;
        }
        else
            if (!flag_ch_added && std::regex_match(std::string(1, ch), std::regex(R"([a-zA-z0-9_])")))
                /* If the first parentheses has not been reached yet but this is a name compatable character append it to the buffer (could be the name of a function) */
                buffer += ch;
            else{
                /* Add the character to the flattened command */
                flattened_cmd += buffer+ch;
                /* If this is not a name like character and the first parentheses has not been reached yet reset the buffer so the function name is not combined with unrelated characters */
                buffer.clear();
            }
    }

    /* Add the rest of the buffer */
    if (!buffer.empty())
        flattened_cmd += "("+buffer+")";

    return flattened_cmd;
}

Engine Engine::load(std::string line){
    /* Remove spaces from line if a function definition is NOT found
    * This way the description in the function description will not have its spaces removed
    */
    std::smatch match;
    if (std::regex_match(line, match, std::regex(R"(^(\S+\s*\(.*?\))\s*\:\s*(.*(\".*\")*)$)"))){
        assert(match.size() == 4);
        line = std::regex_replace(match[1].str(), std::regex(R"(\s+)"), "")+":"+match[2].str();
    }
    else{
        line  = std::regex_replace(line, std::regex(R"(\s+)"), "");
        /* Remove all comments
        * RegEx to remove everything enclosed between IGNORE_CHAR using a lazy match
        */
        line = std::regex_replace(line, std::regex(R"(\)"+IGNORE_CHAR+"(.*?)"+R"(\)"+IGNORE_CHAR), "", std::regex_constants::match_any);
    }

    /* Split by SEP_CHAR and add line to command queue */
    if (line.find(SEP_CHAR) == std::string::npos)
        this->_cmdBuffer.push_back(line);
    else{
        /* Split expression by SEP_CHAR and push into the cmd stack */
        std::stringstream ss(line);
        std::string data;
        std::vector<std::string> cmd_stack;
        while (!ss.eof()){
            std::getline(ss, data, SEP_CHAR);
            cmd_stack.push_back(data);
        }
        for (std::string cmd : cmd_stack)
            this->_cmdBuffer.push_back(cmd);
    }

    return *this;
}

Engine Engine::eval(void){
    /* Check if the command buffer is empty */
    if (this->_cmdBuffer.empty())
        return *this;

    /* Clear the eval buffer */
    this->_evalBuffer.clear();
    /* Reset any error messages */
    this->_error_message.clear();
    this->_warning_message.clear();
    /* Reset the eval wiper */
    this->_evalWiper = 0;

    /* Check for special commands */
    std::smatch match;
    if (this->_cmdBuffer[0] == "help"){
        this->_evalBuffer.push_back(this->help());
        /* Clear command buffer */
        this->_cmdBuffer.clear();
        return *this;
    } else if (this->_cmdBuffer[0] == "report"){
        this->_evalBuffer.push_back(this->report());
        /* Clear command buffer */
        this->_cmdBuffer.clear();
        return *this;
    } else if (this->_cmdBuffer[0] == "reset" || std::regex_match(this->_cmdBuffer[0], match, std::regex(R"(reset#([a-zA-Z0-9_]+))"))){
        /* Check if a function name was given */
        if (match.size() != 2){
            this->_error_message += "[Engine] ERROR: No function name was given for reset operation! Nothing has been reset\n";
            /* Clear command buffer */
            this->_cmdBuffer.clear();
            return *this;
        }
        /* Find the function to be reset */
        auto ref_fun_it = std::find_if(SUPPORTED_FUNS.cbegin(), SUPPORTED_FUNS.cend(), [match](MetaFunction item){return match[1].str() == item.name;});
        if (ref_fun_it == SUPPORTED_FUNS.cend())
            /* If the reference definition could not be found set the error string and return */
            this->_error_message += "[Engine] ERROR: The function `"+match[1].str()+"` could not be reset as it's reference definition could not be found\n";
        else{
            /* If the reference definition was found reset the function definition */
            auto target_fun_it = std::find_if(this->_supported_functions.begin(), this->_supported_functions.end(), [match](MetaFunction item){return match[1].str() == item.name;});
            /* Check if the function definition exists */
            if (target_fun_it != this->_supported_functions.cend()){
                /* If it exists, update it */
                (*target_fun_it).arg_names = (*ref_fun_it).arg_names;
                (*target_fun_it).desc = (*ref_fun_it).desc;
                (*target_fun_it).expr = (*ref_fun_it).expr;
            } else
                /* If it doesn't exist add a new entry */
                this->_supported_functions.push_back(*ref_fun_it);
            this->_error_message += "[Engine] INFO: The function `"+match[1].str()+"` has been successfully reset\n";
        }
        /* Clear command buffer */
        this->_cmdBuffer.clear();
        return *this;
    }

    /* Loop through all commands in queue and evaluate them */
    std::ostringstream str_stream_obj;
    for (std::string cmd : this->_cmdBuffer){
        /* Clear runner's buffers */
        this->_runner.clear();

        /* Check if parentheses are balanced */
        if ((std::count(cmd.cbegin(), cmd.cend(), '(') != std::count(cmd.cbegin(), cmd.cend(), ')'))
            || (std::count(cmd.cbegin(), cmd.cend(), '[') != std::count(cmd.cbegin(), cmd.cend(), ']'))
            || (std::count(cmd.cbegin(), cmd.cend(), '{') != std::count(cmd.cbegin(), cmd.cend(), '}'))){
            this->_error_message += "[Engine] ERROR: Unbalanced parentheses in expression `"+cmd+"`!\n";
            continue;
        }
        /* Check if quotes are balanced */
        if ((std::count(cmd.cbegin(), cmd.cend(), '\'')%2 != 0)
            || (std::count(cmd.cbegin(), cmd.cend(), '"')%2 != 0)){
            this->_error_message += "[Engine] ERROR: Unbalanced quotes in expression `"+cmd+"`!\n";
            continue;
        }

        /* Check for any function definitions using a general definition syntax match
        * RegEx match description:
        *  - Group 1 = Function name
        *  - Group 2 = Arguments
        *  - Group 3 = Expression (with spaces)
        *  - Group 4 = Function description (with spaces, optional)
        */
        std::smatch fun_def_match;
        if (std::regex_match(cmd, fun_def_match, std::regex(R"(^(\S+)\((\S+)\)\:(.*?)\s*(\".*\")*\s*$)"))){
            /* Remove comments and extract the function name */
            std::string fname = std::regex_replace(fun_def_match[1].str(), std::regex(R"(\)"+IGNORE_CHAR+"(.*?)"+R"(\)"+IGNORE_CHAR), "", std::regex_constants::match_any);
            /* Remove comments and extract the function argument(s) */
            std::string fargs = std::regex_replace(fun_def_match[2].str(), std::regex(R"(\)"+IGNORE_CHAR+"(.*?)"+R"(\)"+IGNORE_CHAR), "", std::regex_constants::match_any);
            /* Remove comments and extract the function expression */
            std::string expr = std::regex_replace(fun_def_match[3].str(), std::regex(R"(\)"+IGNORE_CHAR+"(.*?)"+R"(\)"+IGNORE_CHAR), "", std::regex_constants::match_any);
            /* Check if this function has already been defined */
            auto fun_it = std::find_if(this->_supported_functions.begin(), this->_supported_functions.end(), [fname](MetaFunction item){return item.name == fname;});
            if (fun_it != this->_supported_functions.cend()){
                /* Redefinition */
                /* Clear existing argument list */
                (*fun_it).arg_names.clear();
                /* Push in the new argument list */
                size_t split_next = 0;
                size_t split_last = 0;
                while ((split_next = fargs.find(",", split_last)) != std::string::npos) {
                    (*fun_it).arg_names.push_back(fargs.substr(split_last, split_next-split_last));
                    split_last = split_next+1;
                }
                (*fun_it).arg_names.push_back(fargs.substr(split_last));
                /* Update the expression string */
                (*fun_it).expr = std::regex_replace(expr, std::regex(R"(\s+)"), "");
                /* Update the description (if available) */
                (*fun_it).desc = "";
                if (fun_def_match.size() == 5)
                    (*fun_it).desc = fun_def_match[4];
                /* Push an update message to the result buffer */
                this->_evalBuffer.push_back("[Info] Definition for function `"+fname+"` updated");
            } else{
                /* First time definition */
                MetaFunction new_function;
                /* Add the function name */
                new_function.name = fname;
                /* Push in the argument list */
                size_t split_next = 0;
                size_t split_last = 0;
                bool flag_default_arg_start = false;
                while ((split_next = fargs.find(",", split_last)) != std::string::npos) {
                    new_function.arg_names.push_back(fargs.substr(split_last, split_next-split_last));
                    split_last = split_next+1;
                    /* Check if this variable has a default value */
                    if (new_function.arg_names.cend() != std::find(new_function.arg_names.cbegin(), new_function.arg_names.cend(), "="))
                        /* Set the default flag */
                        flag_default_arg_start = true;
                    else
                        /* Check if the default flag is already set */
                        if (flag_default_arg_start){
                            /* A variable with default argument was found before standard arguments */
                            this->_evalBuffer.push_back("[Engine] ERROR: Variable(s) with default argument(s) was found before standard argument(s) in the definition of function `"+new_function.name+"`\n");
                            /* Clear command buffer */
                            this->_cmdBuffer.clear();
                            return *this;
                        }
                }
                new_function.arg_names.push_back(fargs.substr(split_last));
                /* Add the description (if available) */
                new_function.desc = "";
                if (fun_def_match.size() == 5)
                    new_function.desc = fun_def_match[4];
                /* Add the expression string */
                new_function.expr = std::regex_replace(expr, std::regex(R"(\s+)"), "");
                /* Replace known variables with their values */
                new_function.expr = this->replaceVars(new_function.expr);
                /* Verify that only the arguments are remaining */
                this->_runner.parseExpr(new_function.expr);
                std::vector<std::string> infix_buffer = this->_runner.getInfixBuffer();
                for (auto element_it = infix_buffer.cbegin(); element_it != infix_buffer.cend(); ++element_it){
                    /* Check if this is a variable name */
                    if (std::regex_match(*element_it, std::regex(R"([a-zA-Z0-9_]+)")))
                        /* Check if this is an argument */
                        if (new_function.arg_names.cend() == std::find_if(new_function.arg_names.cbegin(), new_function.arg_names.cend(), [element_it](std::string item){return std::regex_search(item.cbegin(), item.cend(), std::regex(*element_it));})){
                            /* Check if this is a function call */
                            if (this->_supported_functions.cend() != std::find_if(this->_supported_functions.cbegin(), this->_supported_functions.cend(), [element_it](MetaFunction item){return *element_it == item.name;}))
                                /* If found confirm that the next element in the buffer is an opening parentheses */
                                if (element_it != infix_buffer.cend()-1 && *(element_it+1) == "(")
                                    /* Good to go this is a valid function call */
                                    continue;
                            /* A variable that is not an argument was found, mark as an error */
                            this->_evalBuffer.push_back("[Engine] ERROR: Unknown variables found in the definition of function `"+new_function.name+"`\n");
                            /* Clear command buffer */
                            this->_cmdBuffer.clear();
                            return *this;
                        }
                }
                this->_runner.clear();
                /* Add the newly created function definition to the supported function list */
                this->_supported_functions.push_back(new_function);
                /* Raise a warning if the function does not have a body */
                if (new_function.expr.empty())
                    this->_warning_message += "[Warning] Function `"+fname+"` does not have a body, it will always return 0 by default!\n";
                /* Push an update message to the result buffer */
                this->_evalBuffer.push_back("[Info] Definition for function `"+fname+"` added");
            }
            /* Clear command buffer */
            this->_cmdBuffer.clear();
            return *this;
        }

        /* Check and replace variables with values in expression
        * This does a blind parse, which results in the parsing done twice on the command
        */
        cmd = this->replaceVars(cmd);

        /* Check for and evaluate any supported function(s) used */
        cmd = this->evalFunctions(cmd);

        /* Check if an assignment operation is present */
        if (cmd.find("=") != std::string::npos){
            /* Split expression by '=' and push into the assignment stack */
            std::stringstream ss(cmd);
            std::string data;
            std::vector<std::string> assignment_stack;
            while (!ss.eof()){
                std::getline(ss, data, '=');
                assignment_stack.push_back(data);
            }

            /* Evaluate the expression to be assigned */
            this->_runner.parseExpr(assignment_stack.back());
            this->_runner.convertToPostfix();
            double val = this->_runner.evaluatePostfix();
            /* Pop the expression from the assignment stack */
            assignment_stack.pop_back();

            /* Assign the result value to all valid variable names */
            for (std::string varName : assignment_stack)
                if (this->checkVarName(varName)){
                    auto indx_it = std::find(this->_varNames.cbegin(), this->_varNames.cend(), varName);
                    /* Make sure that this is a variable and not a function name (in which case the next element will be an opening parentheses)*/
                    if (indx_it != this->_varNames.cend() && !(std::next(indx_it) != this->_varNames.cend() && *(indx_it+1) == "("))
                        /* Update variable value */
                        this->_varValues[indx_it-this->_varNames.cbegin()] = val;
                    else{
                        /* Add new variable */
                        this->_varNames.push_back(varName);
                        this->_varValues.push_back(val);
                    }
                }

            /* Push the result to the result buffer
            * Using a string stream allows for a cleaner number representation
            * when converted to string unlike std::to_string().
            */
            str_stream_obj << val;
            this->_evalBuffer.push_back(str_stream_obj.str());
            /* Clear the string stream */
            str_stream_obj.str("");
            str_stream_obj.clear();
        }
        else{
            /* Parse expression */
            this->_runner.parseExpr(cmd);
            this->_runner.convertToPostfix();
            /* Evaluate the expression and push it into the result queue
            * Using a string stream allows for a cleaner number representation
            * when converted to string unlike std::to_string().
            */
            str_stream_obj << this->_runner.evaluatePostfix();
            this->_evalBuffer.push_back(str_stream_obj.str());
            /* Clear the string stream */
            str_stream_obj.str("");
            str_stream_obj.clear();
        }
    }
    /* Clear command buffer */
    this->_cmdBuffer.clear();

    return *this;
}

const std::string Engine::getResult(void){
    if (this->_evalWiper >= this->_evalBuffer.size())
        return RESULT_END;
    
    /* Return result and increment the wiper */
    return this->_evalBuffer[this->_evalWiper++];
}

const std::string Engine::help(void){
    std::string help_str = "-- Help for MB Compute Engine version "+ENGINE_VERSION+" --\n";

    /* Add help for supported commands */
    help_str += "Supported commands:\n";
    help_str += "   - help -----------------> Return this message\n";
    help_str += "   - reset #function_name -> Reset an inbuilt function's definition\n";
    help_str += "                           does nothing if given function is not an inbuilt function\n";
    help_str += "   - report ---------------> Return a summary all the defined variables and functions\n";
    help_str += "\n";

    /* Add help for supported operators */
    help_str += "Supported operators:\n";
    for (MetaOperator imo : SUPPORTED_OOPS)
        help_str += imo.cat+" "+imo.desc+" [Precedence: "+std::string(1, imo.order)+"]: "+imo.oop+"\n";
    help_str += "\n";

    /* Add help for supported functions */
    help_str += "Supported functions:\n";
    for (MetaFunction imf : this->_supported_functions){
        help_str += "   * "+imf.name+"(";
        for (std::string arg_name :  imf.arg_names)
            help_str += arg_name+" "+IGNORE_CHAR+FUNCTION_ARG_TYPE+IGNORE_CHAR+", ";
        /* Remove the extra trailing space */
        help_str.pop_back();
        /* Remove the extra trailing comma */
        help_str.pop_back();
        help_str += ") : "+imf.expr;
        if (imf.desc.empty())
            help_str += "\n";
        else
            help_str += " "+IGNORE_CHAR+imf.desc+IGNORE_CHAR+"\n";
    }
    help_str += "\n";

    /* Add details on comments */
    help_str += "Comments:\n";
    help_str += "A comment is defined by enclosing any text/expression in `"+IGNORE_CHAR+"`\n";
    help_str += "Example(s)-\n";
    help_str += "   1. Using a comment.\n";
    help_str += "       MB> "+IGNORE_CHAR+"This is a comment"+IGNORE_CHAR+"\n";
    help_str += "   2. Using a comment in the meddle of an expression.\n";
    help_str += "       MB> a "+IGNORE_CHAR+"This is also comment"+IGNORE_CHAR+" =5*10\n";
    help_str += "\n";

    /* Add details on the separation character */
    help_str += "Separator:\n";
    help_str += "Multiple expressions and assignments can be combined into a single line using `"+std::to_string(SEP_CHAR)+"`\n";
    help_str += "   Combined expressions will be evaluated from left to right\n";
    help_str += "Note that this feature is not supported for function definitions!\n";
    help_str += "Example(s)-\n";
    help_str += "   1. Using the separator.\n";
    help_str += "       MB> var1=10"+std::to_string(SEP_CHAR)+"var2=var1+1\n";
    help_str += "       11\n";
    help_str += "       MB> \n";
    help_str += "\n";

    /* Add details on how a function can be defined or overloaded */
    help_str += "Defining/Updating a variable:\n";
    help_str += "A new variable can be defined by using the below syntax.\n";
    help_str += "    my_variable=expression\n";
    help_str += "Where-\n";
    help_str += "   - `my_variable` is the variable name\n";
    help_str += "   - `expression` is the expression who's evaluated result will be stored in the variable\n";
    /* Technically speaking the value is not "stored" in the variable but rather the variable name will be replaced with the value everywhere */
    help_str += "The same syntax can be used on existing variables to update their value\n";
    help_str += "\n";

    /* Add details on how a function can be defined or overloaded
    * 
    * Note on the "overload" part:
    *  Technically the function is being redefined not overloaded.
    *  Meaning the argument and return signatures of the function are also overwritten!
    */
    help_str += "Defining a function:\n";
    help_str += "A new function can be defined by using the below syntax.\n";
    help_str += "    my_function(variable_1, variable_2, ..., variable_n=10) : my_function_expression "+IGNORE_CHAR+"my_function_description"+IGNORE_CHAR+"\n";
    help_str += "Where-\n";
    help_str += "   - `my_function` is the function name\n";
    help_str += "       This MUST start with an alphabet (a-z or A-Z) or an underscore (_)\n";
    help_str += "       then can be followed by any number of alphanumeric or underscore characters (a-z or A-Z or 0-9 or _)\n";
    help_str += "       Note that function names that start and end with double underscores (__) are reserved!\n";
    help_str += "   - `variable_1, variable_2, ..., variable_n=10` is the list of variables that the function will take as arguments\n";
    help_str += "       All arguments WILL be intrepreted as doubles (8-byte floating point number), explicit type definitions are NOT supported.\n";
    help_str += "       Note that if the function is called with out the required number of arguments an error will be raised.\n";
    help_str += "       To assign a default value for an argument the assignment operator (=) can be used just after the variable name.\n";
    help_str += "       Note that all variables with default values MUST be at the end of the argument list otherwise an error will be raised.\n";
    help_str += "       Note that if any external variables are used their value will be evaluated during the function's definition and so any updates to the external variable will not effect the function.\n";
    help_str += "   - `my_function_expression` is the expression the function will implement\n";
    help_str += "       Note that if all variables used in this expression MUST be in the argument list of be previously defined.\n";
    help_str += "   - `my_function_description` is the description of the function\n";
    help_str += "       This is an optional part of the function definiton syntax, if omitted the help command will show an empty description.\n";
    help_str += "       Note that multiline descriptions are NOT supported.\n";
    help_str += "Example(s)-\n";
    help_str += "   1. Defining a simple function without a description string.\n";
    help_str += "       MB> fun(var1) : var1+1\n";
    help_str += "       MB> fun(10)\n";
    help_str += "       11\n";
    help_str += "       MB> \n";
    help_str += "   2. Defining a simple function with a description string.\n";
    help_str += "       MB> fun(var1) : var1+1 "+IGNORE_CHAR+"Add 1 to argument"+IGNORE_CHAR+"\n";
    help_str += "       MB> fun(10)\n";
    help_str += "       11\n";
    help_str += "       MB> \n";
    help_str += "   3. Defining a function that uses a predefined variable.\n";
    help_str += "       MB> num=5\n";
    help_str += "       MB> fun(var1) : var1*num "+IGNORE_CHAR+"Multiply argument with num"+IGNORE_CHAR+"\n";
    help_str += "       MB> fun(10)\n";
    help_str += "       50\n";
    help_str += "       MB> \n";
    help_str += "   4. Defining a function with variables that have default values \n";
    help_str += "       MB> fun(var1,var2,var3=3) : (var1+var2)*var3 "+IGNORE_CHAR+"Multiply 3rd argument with sum of the first 2"+IGNORE_CHAR+"\n";
    help_str += "       MB> fun(1, -5)\n";
    help_str += "       -12\n";
    help_str += "       MB> fun(-1, 5, 2)\n";
    help_str += "       8\n";
    help_str += "       MB> \n";
    help_str += "\n";
    help_str += "Redefining functions:\n";
    help_str += "Any and all functions can be redefined by simply using the function definiton syntax on the existing function name.\n";
    help_str += "Note that this redefining process is not OVERLOADING as the argument list will also be overwritten to the new definition!\n";

    return help_str;
}

const std::string Engine::report(void){
    std::string report_str = "-- Summary of current environment --\n";

    /* List all defined functions */
    report_str += IGNORE_CHAR+"Defined functions:"+IGNORE_CHAR+"\n";
    if (this->_supported_functions.empty())
        report_str += "   "+IGNORE_CHAR+"NO FUNCTIONS DEFINED"+IGNORE_CHAR+"\n";
    else
        for (MetaFunction imf : this->_supported_functions){
            report_str += "   "+imf.name+"(";
            for (std::string arg_name :  imf.arg_names)
                report_str += arg_name+" "+IGNORE_CHAR+FUNCTION_ARG_TYPE+IGNORE_CHAR+", ";
            /* Remove the extra trailing space */
            report_str.pop_back();
            /* Remove the extra trailing comma */
            report_str.pop_back();
            report_str += ") : "+imf.expr;
            if (imf.desc.empty())
                report_str += "\n";
            else
                report_str += " "+IGNORE_CHAR+imf.desc+IGNORE_CHAR+"\n";
        }
    report_str += "\n";

    /* List all declared variables */
    report_str += IGNORE_CHAR+"Declared variables:"+IGNORE_CHAR+"\n";
    if (this->_varNames.empty())
        report_str += "   "+IGNORE_CHAR+"NO VARIABLES DECLARED"+IGNORE_CHAR+"\n";
    else
        for (size_t index = 0; index < this->_varNames.size(); ++index)
            report_str += "   "+this->_varNames[index]+"="+std::to_string(this->_varValues[index])+"\n";

    return report_str;
}

const std::string Engine::getErrorMsg(void)
{
    if (!this->_runner.getErrorMsg().empty())
        return this->_runner.getErrorMsg()+this->_error_message;
    else
        return this->_error_message;
}

const std::string Engine::getWarningMsg(void)
{
    if (!this->_runner.getWarningMsg().empty())
        return this->_runner.getWarningMsg()+this->_warning_message;
    else
        return this->_warning_message;
}

/* Evaluator class definitions */
Evaluator::Evaluator(const std::string expression){
    this->parseExpr(expression);
}

Evaluator::Evaluator(void){
    /* Extract the highest precedence of all supported operators */
    this->_max_precedence = 0;
    for (MetaOperator mo : SUPPORTED_OOPS)
        if (this->_max_precedence < mo.order)
            this->_max_precedence = mo.order;
}

Evaluator::~Evaluator(void){
}

const std::vector<std::string> Evaluator::getInfixBuffer(void){
    return this->_expression_infix;
}

const std::vector<std::string> Evaluator::getPostfixBuffer(void){
    return this->_expression_postfix;
}

const std::string Evaluator::getErrorMsg(void){
    return this->_error_message;
}

const std::string Evaluator::getWarningMsg(void){
    return this->_warning_message;
}

int Evaluator::getOPP(std::string opr){
    auto itr = std::find_if(SUPPORTED_OOPS.cbegin(), SUPPORTED_OOPS.cend(), [opr](MetaOperator item){return opr == item.oop;});
    /* Check if the requested operator was found */
    if (itr != SUPPORTED_OOPS.cend())
        return this->_max_precedence+1 - (*itr).order;
    else
        return 0;
}

/* SI prefixes
* +-------+--------+------------+-----------------------------+-----------------------+
* | Name  | Symbol | Scientific |          Decimal            | English (Short scale) |
* +-------+--------+------------+-----------------------------+-----------------------+
* | yotta |  Y     | 1E+24      |  1000000000000000000000000  |  septillion           |
* | zetta |  Z     | 1E+21      |  1000000000000000000000     |  sextillion           |
* | exa   |  E     | 1E+18      |  1000000000000000000        |  quintillion          |
* | peta  |  P     | 1E+15      |  1000000000000000           |  quadrillion          |
* | tera  |  T     | 1E+12      |  1000000000000              |  trillion             |
* | giga  |  G     | 1E+9       |  1000000000                 |  billion              |
* | mega  |  M     | 1E+6       |  1000000                    |  million              |
* | kilo  |  k     | 1E+3       |  1000                       |  thousand             |
* | hecto |  h     | 1E+2       |  100                        |  hundred              |
* | deca  |  da    | 1E+1       |  10                         |  ten                  |
* |       |        | 1E+0       |  1                          |  one                  |
* | deci  |  d     | 1E-1       |  0.1                        |  tenth                |
* | centi |  c     | 1E-2       |  0.01                       |  hundredth            |
* | milli |  m     | 1E-3       |  0.001                      |  thousandth           |
* | micro |  μ/u   | 1E-6       |  0.000001                   |  millionth            |
* | nano  |  n     | 1E-9       |  0.000000001                |  billionth            |
* | pico  |  p     | 1E-12      |  0.000000000001             |  trillionth           |
* | femto |  f     | 1E-15      |  0.000000000000001          |  quadrillionth        |
* | atto  |  a     | 1E-18      |  0.000000000000000001       |  quintillionth        |
* | zepto |  z     | 1E-21      |  0.000000000000000000001    |  sextillionth         |
* | yocto |  y     | 1E-24      |  0.000000000000000000000001 |  septillionth         |
* +-------+--------+------------+-----------------------------+-----------------------+
*/
Evaluator Evaluator::parseExpr(std::string expr){
    /* Remove spaces in input before proceeding */
    expr.erase(std::remove_if(expr.begin(), expr.end(), [](unsigned char x) { return std::isspace(x); }), expr.cend());

    /* flag to mark if a number is being parsed */
    bool flag_number = false;
    bool flag_sci_skip = false;
    bool flag_variable = false;

    /* Iterate input expression */
    unsigned int bracket_count = 0;
    for (std::string::const_iterator it = expr.cbegin(); it != expr.cend(); ++it){
        if (not std::isdigit(*it))
            if (flag_number){
                /* If the current character is E */
                /* Convert SI prefixes to their scientific from */
                int search_start_offset = 10;
                int search_end_offset = 5;
                switch (*it){
                    case 'Y':
                        this->_expression_infix.back() += "*(1E+24)";
                        break;
                    case 'Z':
                        this->_expression_infix.back() += "*(1E+21)";
                        break;
                    case 'E':
                        /* the next character is + or - then followed by a  then this is part is a scientific formatted number
                        * skip replacement
                        */
                        search_start_offset = 10;
                        search_end_offset = 5;
                        if (it-expr.cbegin() < search_start_offset)
                            search_start_offset = it-expr.cbegin();
                        if (expr.cend()-it < search_end_offset)
                            search_end_offset = expr.cend()-it;

                        if (std::regex_search(std::string(it-search_start_offset, it+search_end_offset), std::regex("[\\.0-9]+(E[\\-\\+][0-9]+)"))){
                            this->_expression_infix.back() += "E";
                            /* Set flag to skip the following +/- character */
                            flag_sci_skip = true;
                        }
                        else
                            this->_expression_infix.back() += "*(1E+28)";
                        break;
                    case 'P':
                        this->_expression_infix.back() += "*(1E+15)";
                        break;
                    case 'T':
                        this->_expression_infix.back() += "*(1E+12)";
                        break;
                    case 'G':
                        this->_expression_infix.back() += "*(1E+9)";
                        break;
                    case 'M':
                        this->_expression_infix.back() += "*(1E+6)";
                        break;
                    case 'k':
                        this->_expression_infix.back() += "*(1E+3)";
                        break;
                    case 'h':
                        this->_expression_infix.back() += "*(1E+2)";
                        break;
                    // case 'da':
                    //     this->_expression_infix.back() += "*(1E+1)";
                    //     break;
                    case 'd':
                        if (*(it+1) == 'a'){
                            /* Case for da */
                            this->_expression_infix.back() += "*(1E+1)";
                            /* Skip next character */
                            ++it;
                        }
                        else
                            /* Case for d */
                            this->_expression_infix.back() += "*(1E-1)";
                        break;
                    case 'c':
                        this->_expression_infix.back() += "*(1E-2)";
                        break;
                    case 'm':
                        this->_expression_infix.back() += "*(1E-3)";
                        break;
                    case 'u':
                        this->_expression_infix.back() += "*(1E-6)";
                        break;
                    case 'n':
                        this->_expression_infix.back() += "*(1E-9)";
                        break;
                    case 'p':
                        this->_expression_infix.back() += "*(1E-12)";
                        break;
                    case 'f':
                        this->_expression_infix.back() += "*(1E-15)";
                        break;
                    case 'a':
                        this->_expression_infix.back() += "*(1E-18)";
                        break;
                    case 'z':
                        this->_expression_infix.back() += "*(1E-21)";
                        break;
                    case 'y':
                        this->_expression_infix.back() += "*(1E-24)";
                        break;
                    case '.':
                        this->_expression_infix.back() += *it;
                        break;
                    default:
                        /* Extend the +/- charater of a scientific formatted number */
                        if (flag_sci_skip && (*it == '+' || *it == '-')){
                            this->_expression_infix.back() += *it;
                            /* Reset the skip flag */
                            flag_sci_skip = false;
                        } else{
                            /* Number has ended and a operator has started! */
                            /* Remove trailing opening bracket if nothing has been added since */
                            if (this->_expression_infix.back() == "(")
                                this->_expression_infix.pop_back();
                            else
                                this->_expression_infix.push_back(")");
                            bracket_count--;
                            /* Add the operator */
                            this->_expression_infix.push_back(std::string(1, *it));
                            /* Check if this is a two character operator */
                            if (SUPPORTED_OOPS.cend() != std::find_if(SUPPORTED_OOPS.cbegin(), SUPPORTED_OOPS.cend(), [it](MetaOperator item){return item.oop == (*it)+std::string(1, *(it+1));}))
                                this->_expression_infix.back() += std::string(1, *(++it));
                            /* Mark the end of a number */
                            flag_number = false;
                        }
                        break;
                }
                flag_variable = false;
            } else if (std::isalpha(*it) || *it == '_'){
                if (flag_variable)
                    /* If this is part of a variable name append to previous */
                    this->_expression_infix.back() += *it;
                else{
                    /* If this is the start of a variable name append */
                    this->_expression_infix.push_back(std::string(1, *it));
                    flag_variable = true;
                }
                flag_number = false;
            } else{
                /* Check if this is a negative number and
                * make sure that this is a negative number and not an operator following a bracket or followed by a number/variable
                */
                if (*it == '-' && isdigit(*(it+1)) && (it == expr.cbegin() || (*(it-1) != ')' && !flag_number && !flag_variable))){
                    flag_number = true;
                    if (*(it-1) != ')'){
                        this->_expression_infix.push_back("(");
                        bracket_count++;
                    }
                }
                else
                    flag_number = false;
                flag_variable = false;
                /* Append all other characters as is */
                this->_expression_infix.push_back(std::string(1, *it));
                /* Check if this is a two character operator */
                if (SUPPORTED_OOPS.cend() != std::find_if(SUPPORTED_OOPS.cbegin(), SUPPORTED_OOPS.cend(), [it](MetaOperator item){return item.oop == (*it)+std::string(1, *(it+1));}))
                    this->_expression_infix.back() += std::string(1, *(++it));
            }
        else{
            if (flag_number || flag_variable)
                /* Accumulate the characters of the same number/variable */
                this->_expression_infix.back() += *it;
            else{
                /* Add a final closing bracket to the previous section if needed */
                if (bracket_count == 1){
                    /* Remove trailing opening bracket if nothing has been added since */
                    if (this->_expression_infix.back() == "(")
                        this->_expression_infix.pop_back();
                    else
                        this->_expression_infix.push_back(")");
                    bracket_count--;
                }
                /* Append to list if this is the first character of the number */
                this->_expression_infix.push_back("(");
                bracket_count++;
                this->_expression_infix.push_back(std::string(1, *it));
                flag_number = true;
                flag_variable = false;
            /* Reinforce that both the number and variable flags are not the same */
            assert(flag_number != flag_variable);
            }
        }
    }

    /* Add a final closing bracket if needed */
    if (bracket_count == 1){
        /* Remove trailing opening bracket if nothing has been added since */
        if (this->_expression_infix.back() == "(")
            this->_expression_infix.pop_back();
        else
            this->_expression_infix.push_back(")");
        bracket_count--;
    }

    /* Reinforce that no extra brackets exist */
    assert(bracket_count == 0);

    /* Return back object so methods can be cascaded */
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

    for (auto it = this->_expression_infix.cbegin(); it != this->_expression_infix.cend(); ++it){
        /* If scanned character is open bracket push it on stack */
        if(*it == "(" || *it == "[" || *it == "{")
            stack.push(*it);
        /* If scanned character is opened bracket pop all literals from stack till matching open bracket gets popped */
        else if(*it == ")" || *it == "]" || *it == "}"){
            if(*it == ")")
                while(stack.top() != "("){
                    this->_expression_postfix.push_back(stack.top());
                    stack.pop();
                }
            else if(*it == "]")
                while(stack.top() != "["){
                    this->_expression_postfix.push_back(stack.top());
                    stack.pop();
                }
            else if(*it == "}")
                while(stack.top() != "{"){
                    this->_expression_postfix.push_back(stack.top());
                    stack.pop();
                }
            stack.pop();
        } else if(SUPPORTED_OOPS.cend() != std::find_if(SUPPORTED_OOPS.cbegin(), SUPPORTED_OOPS.cend(), [it](MetaOperator item){return item.oop == *it;}))
            /* If scanned character is operator */
            /* very first operator of expression is to be pushed on stack */
            if(stack.empty())
                stack.push(*it);
            else
                /* Check the precedence order of instack(means the one on top of stack) and incoming operator,
                * if instack operator has higher priority than incoming operator pop it out of stack&put it in
                * final postfix expression, on other side if precedence order of instack operator is less than i
                * coming operator, push incoming operator on stack.
                */
                if(getOPP(stack.top()) >= getOPP(*it)){
                    this->_expression_postfix.push_back(stack.top());
                    stack.pop();
                    stack.push(*it);
                } else
                    stack.push(*it);
        else
            /* If literal is operand, put it on to final postfix expression */
            this->_expression_postfix.push_back(*it);
    }

    /* Popping out all remaining operator literals & adding to final postfix expression */
    if(!stack.empty()){
        while(!stack.empty()){
            this->_expression_postfix.push_back(stack.top());
            stack.pop();
        }
    }

    /* Return back object so methods can be cascaded */
    return *this;
}

void Evaluator::clear(void){
    this->_error_message.clear();
    this->_warning_message.clear();
    this->_expression_infix.clear();
    this->_expression_postfix.clear();
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
    for (auto it = this->_expression_postfix.cbegin(); it != this->_expression_postfix.cend(); ++it){
        /* If the scanned element is an operand (number), push it to the stack. */
        if (isdigit((*it)[0]) || ((*it)[0] == '-' && (*it).length() > 1 && isdigit((*it)[1]))){
            stack.push(std::atof((*it).c_str()));
        }
        /* Process alphabets/variables */
        else if (isalpha((*it)[0]) || (*it)[0] == '_'){
            return 0;
        }
        /* If the scanned element is an operator, pop two
        * elements from stack to apply the operator
        */
        else{
            /* If the stack is empty set the error message and return 0 */
            if (stack.empty()){
                this->_error_message += "[Evaluator] ERROR: No operands where given to the operator "+*it+"!\n";
                return 0;
            }
            double val1 = stack.top();
            stack.pop();
            /* Don't pop another number if this is a single number operation */
            double val2 = 0;
            if (!(*it == "++" || *it == "--" || *it == "!")){
                /* If the stack is empty set the error message and return 0 */
                if (stack.empty()){
                    /* Check if this is a supported operator */
                    if (SUPPORTED_OOPS.cend() == std::find_if(SUPPORTED_OOPS.cbegin(), SUPPORTED_OOPS.cend(), [it](MetaOperator item){return item.oop == *it;}))
                        this->_error_message += "[Evaluator] ERROR: Unsupported operator `"+*it+"`! You can use the `help` command to get a list of supported operators.\n";
                    else
                        this->_error_message += "[Evaluator] ERROR: Operator "+*it+" requires 2 operands however only one ("+std::to_string(val1)+") was given!\n";
                    return 0;
                }
                val2 = stack.top();
                stack.pop();
            }

            if (*it == "++")
                stack.push(val1 + 1);
            else if (*it == "+")
                stack.push(val2 + val1);
            else if (*it == "--")
                stack.push(val1 - 1);
            else if (*it == "-")
                stack.push(val2 - val1);
            else if (*it == "**")
                stack.push(std::pow(val2, val1));
            else if (*it == "*")
                stack.push(val2 * val1);
            else if (*it == "/")
                stack.push(val2 / val1);
            else if (*it == "%")
                stack.push(std::fmod(val2, val1));
            else if (*it == "^^")
                stack.push(static_cast<double>(!val2 != !val1));
            else if (*it == "^")
                stack.push(static_cast<double>(static_cast<long long>(val2) ^ static_cast<long long>(val1)));
            else if (*it == "||")
                stack.push(static_cast<double>(val2 || val1));
            else if (*it == "|")
                stack.push(static_cast<double>(static_cast<long long>(val2) | static_cast<long long>(val1)));
            else if (*it == "&&")
                stack.push(static_cast<double>(val2 && val1));
            else if (*it == "&")
                stack.push(static_cast<double>(static_cast<long long>(val2) & static_cast<long long>(val1)));
            else if (*it == "<<")
                stack.push(static_cast<double>(static_cast<long long>(val2) << static_cast<long long>(val1)));
            else if (*it == "<")
                stack.push(static_cast<double>(val2 < val1));
            else if (*it == ">>")
                stack.push(static_cast<double>(static_cast<long long>(val2) >> static_cast<long long>(val1)));
            else if (*it == ">")
                stack.push(static_cast<double>(val2 > val1));
            else if (*it == "!=")
                stack.push(static_cast<double>(val2 != val1));
            else if (*it == "!")
                stack.push(static_cast<double>(!val1));
            else if (*it == "==")
                stack.push(static_cast<double>(val2 == val1));
        }
    }

    /* Raise a warning if the stack has multiple results */
    if (stack.size() > 1)
        this->_warning_message += "[Evaluator] WARNING: Multiple results in stack!\n";

    /* Return 0 if the stack is empty */
    if (stack.empty())
        return 0;
    else
        return stack.top();
}

const std::string getRegExEscaped(const std::string str){
    /* Regex escape
    * Matches any characters that need to be escaped in RegEx
    */
    std::regex regExSpecialChars { R"([-[\]{}()*+?.,\^$|#\s])" };
    return std::regex_replace( str, regExSpecialChars, R"(\$&)" );
}

}
