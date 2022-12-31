/****************************************************************************
* File name: mb_compute_core.cpp
* Version: v1.0.1
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
*  Console interface for the MB compute engine.
****************************************************************************/

/* Includes */
#include <iostream>
#include <string>
#include <signal.h>

/* Custom libraties */
#include "mbcomputengine_lib.hpp"
#include "mbcsupport_lib.hpp"

/* Common definitions */
#define CONSOLE_READY_MSG "MB> "

/* Global variable to old current verbosity level */
int verbose = 0;

/* Function to print based on verbosity level */
void print_verbose(std::string msg, int verbosity=1){
    if (verbose >= verbosity)
        std::cout << msg << std::endl;
}

/* Callback function to handle console/system signals sent to application
 * Reference link: https://www.cplusplus.com/reference/csignal/signal/
 * +---------+---------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
 * | Signal  | Signal name                     | Description                                                                                                                                  |
 * +---------+---------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
 * | SIGABRT | Abort signal                    | Abnormal termination, such as is initiated by the abort function.                                                                            |
 * | SIGFPE  | Floating-Point Exception signal | Erroneous arithmetic operation, such as zero divide or an operation resulting in overflow (not necessarily with a floating-point operation). |
 * | SIGILL  | Illegal Instruction signal      | Invalid function image, such as an illegal instruction. This is generally due to a corruption in the code or to an attempt to execute data.  |
 * | SIGINT  | Interrupt signal                | Interactive attention signal. Generally generated by the application user.                                                                   |
 * | SIGSEGV | Segmentation Violation signal   | Invalid access to storage: When a program tries to read or write outside the memory it has allocated.                                        |
 * | SIGTERM | Terminate signal                | Termination request sent to program.                                                                                                         |
 * +---------+---------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------+
 */
void signal_callback_handler(int signal_num) {
   switch (signal_num)
   {
        case SIGABRT:
            std::cerr << std::endl << "[ERROR] An Abort signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
        case SIGFPE:
            std::cerr << std::endl << "[ERROR] An Floating-Point Exception signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
        case SIGILL:
            std::cerr << std::endl << "[ERROR] An Illegal Instruction signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
        case SIGINT:
            /* Handle and continue if a console interrupt is received */
            std::cout << std::endl;
            std::cerr << "[WARNING] An Interrupt signal was raised by OS" << std::endl;
            std::cout << CONSOLE_READY_MSG;
            std::cout.flush();
            // exit(signal_num);
            break;
        case SIGSEGV:
            std::cerr << std::endl << "[ERROR] An Segmentation Violation signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
        case SIGTERM:
            std::cerr << std::endl << "[ERROR] An Terminate signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
        default:
            std::cerr << std::endl << "[ERROR] An unknown signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
   }
}

/* Function to perform final cleanup before exiting */
void self_cleanup(void){
    std::cout << "Exiting..." << std::endl;
}

int main(int argc, char *argv[]){
    /* Register signal and signal handler */
    if (signal(SIGABRT, signal_callback_handler) == SIG_ERR
        or signal(SIGFPE, signal_callback_handler) == SIG_ERR
        or signal(SIGILL, signal_callback_handler) == SIG_ERR
        or signal(SIGINT, signal_callback_handler) == SIG_ERR
        or signal(SIGSEGV, signal_callback_handler) == SIG_ERR
        or signal(SIGTERM, signal_callback_handler) == SIG_ERR){
        std::cerr << "[ERROR] [ERROR_CODE=" << errno << "] Core signal handlers could not be registered" << std::endl;
    }

    /* Init compute engine */
    mbc::Engine eng;

    /* Console input buffer */
    std::string input;

    /* Console interface input loop */
    // Test string: 12.503+15.43*12-(2m + 5M) >> -4.9998e+06
    mbc::Evaluator test_eng;
    do
    {
        /* Display ready message and wait for user input */
        std::cout << CONSOLE_READY_MSG;
        std::cout.flush();
        std::cin.clear();
        std::getline(std::cin, input);

        /* Skip processing if input is empty */
        if (input.empty()){
            std::cout << "[INFO] Empty input" << std::endl;
            continue;
        } else if (input == "exit")
            break;

        test_eng.parseExpr(input);
        std::cout << "[DEBUG] Parsed: " << mbcs::get_printable_vector(test_eng.getInfixBuffer()) << std::endl;

        test_eng.convertToPostfix();
        std::cout << "[DEBUG] Postfix: " << mbcs::get_printable_vector(test_eng.getPostfixBuffer()) << std::endl;
        std::cout << "[DEBUG] Result: " << test_eng.evaluatePostfix() << std::endl;

        /* Clear buffers */
        test_eng.clear();
    } while (input != "exit");

    /* Perform all cleanup duties before exiting */
    self_cleanup();
    return 0;
}
