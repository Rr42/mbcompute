/****************************************************************************
* File name: mb_compute_core.cpp
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
*  Console interface for the MB compute engine.
****************************************************************************/

/* Includes */
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <signal.h>

/* Custom libraries */
#include "mbcomputengine_lib.hpp"
#include "mbcsupport_lib.hpp"

/* Common definitions */
#define CONSOLE_READY_MSG "MB> "

/* Global variable to hold the current verbosity level */
int verbose = 0;
std::string session_history = "";

/* Class to handle printing and logging
* Reference link: https://stackoverflow.com/a/14155788/7261761
*/
class log_stream{
private:
    std::ofstream fout;
    bool flag_log_set;

public:
    log_stream(std::string log_file = ""){
        // Check if opening the file succeeded
        if (log_file.empty()){
            this->flag_log_set = false;
        } else{
            this->fout.open(log_file);
            if (this->fout.good())
                this->flag_log_set = true;
            else
                this->flag_log_set = false;
        }
    };
    ~log_stream(){
        /* Cleanup */
        if (this->fout){
            /* Flush all streams */
            this->flush();
            /* Close the log file stream */
            this->fout.close();
        }
    };
    void open(std::string log_file){
        /* Close existing handle */
        if (this->fout)
            this->fout.close();
        /* Open new handle */
        this->fout.open(log_file);
        if (this->fout.good())
            this->flag_log_set = true;
        else
            this->flag_log_set = false;
    }
    void flush(){
        std::cout.flush();
        if (this->flag_log_set)
            this->fout.flush();
    }
    void log(std::string message){
        if (this->flag_log_set)
            this->fout << message;
    }
    // For regular output of variables and stuff
    template<typename T> log_stream& operator<<(const T& something){
        std::cout << something;
        if (this->flag_log_set)
            fout << something;
        return *this;
    }
    // For manipulators like std::endl
    typedef std::ostream& (*stream_function)(std::ostream&);
    log_stream& operator<<(stream_function func){
        func(std::cout);
        if (this->flag_log_set)
            func(fout);
        return *this;
    }
} flog;

/* Function to print based on verbosity level */
void print_verbose(std::string msg, int verbosity=1){
    if (verbose >= verbosity)
        flog << msg << std::endl;
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
            std::cerr << std::endl << "[ERROR] A Floating-Point Exception signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
        case SIGILL:
            std::cerr << std::endl << "[ERROR] An Illegal Instruction signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
        case SIGINT:
            /* Handle and continue if a console interrupt is received */
            flog << std::endl;
            std::cerr << "[WARNING] An Interrupt signal was raised by OS" << std::endl;
            flog << CONSOLE_READY_MSG;
            flog.flush();
            // exit(signal_num);
            break;
        case SIGSEGV:
            std::cerr << std::endl << "[ERROR] A Segmentation Violation signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
        case SIGTERM:
            std::cerr << std::endl << "[ERROR] A Terminate signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
        default:
            std::cerr << std::endl << "[ERROR] An unknown signal was raised by OS" << std::endl;
            exit(signal_num);
            break;
   }
}

/* Function to perform final cleanup before exiting */
void self_cleanup(bool silentFlag = false){
    if (!silentFlag)
        flog << "Exiting..." << std::endl;
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

    /* Handle CLI flags and options */
    mbcs::CLIParser CLIparser(argc, argv);
    bool pipeFlag = false;
    bool silentFlag = false;
    std::string command = "";
    std::string log_file = "";
    if (CLIparser.cmdOptionExists("-h") || CLIparser.cmdOptionExists("--help")){
        flog << "Usage mbconsole [OPTIONS]" << std::endl;
        flog << std::endl;
        flog << "Options:" << std::endl;
        flog << "  -h, --help           Show this message" << std::endl;
        flog << "                       (This option will take the highest precedence)" << std::endl;
        flog << "  -p, --piped-input    Operates in piped input/output mode" << std::endl;
        flog << "  -s, --silent         Operates in silent mode" << std::endl;
        flog << "  -l=s, --log=s        Saves all terminal interactions to given log file" << std::endl;
        flog << "  -c=s, --command=s    Executes given command before continuing" << std::endl;
        /* Perform all cleanup duties and exiting */
        self_cleanup();
        return 0;
    }
    if (CLIparser.cmdOptionExists("--silent") || CLIparser.cmdOptionExists("-s"))
        silentFlag = true;
    if (CLIparser.cmdOptionExists("--piped-input") || CLIparser.cmdOptionExists("-p"))
        pipeFlag = true;
    if (CLIparser.cmdOptionExists("--command")){
        command = CLIparser.getCmdOption("--command");
        /* Remove the option part */
        command.erase(0, 10);
    } else if (CLIparser.cmdOptionExists("-c")){
        command = CLIparser.getCmdOption("-c");
        /* Remove the option part */
        command.erase(0, 3);
    }
    if (CLIparser.cmdOptionExists("--log")){
        log_file = CLIparser.getCmdOption("--log");
        /* Remove the option part */
        log_file.erase(0, 6);
    } else if (CLIparser.cmdOptionExists("-l")){
        log_file = CLIparser.getCmdOption("-l");
        /* Remove the option part */
        log_file.erase(0, 3);
    }
    if (!command.empty()){
        command = std::regex_replace(command, std::regex("\\\\n"), "\n");
        if (!std::regex_search(command, std::regex("\n$")))
            command += "\n";
    }

    /* Init compute engine */
    mbc::Engine eng;
    std::string result_old;
    std::string result;

    /* Init log file */
    flog.open(log_file);

    /* Process the given commands one line at a time */
    if (!command.empty()){
        /* If the command has multiple lines */
        std::size_t pos = 0;
        std::string cmd_line;
        while ((pos = command.find("\n")) != std::string::npos) {
            cmd_line = command.substr(0, pos);
            /* Check the line is an exit command */
            if (cmd_line == "exit"){
                /* If so cleanup and exit */
                self_cleanup(silentFlag);
                return 0;
            }
            /* Load and execute the line */
            eng.load(cmd_line);
            eng.eval();
            /* Check and print any warnings */
            if (!eng.getWarningMsg().empty())
                flog << eng.getWarningMsg();
            /* Check for any errors if there are none print the result to output */
            if (!eng.getErrorMsg().empty())
                flog << eng.getErrorMsg();
            else{
                /* Get the last result */
                while ((result = eng.getResult()) != mbc::RESULT_END)
                    result_old = result;
                flog << result_old << std::endl;
            }
            /* Clear command buffer for reading the next line */
            command.erase(0, pos + 1);
        }
    }

    /* Console input buffer */
    std::string input;

    /* Console interface input loop */
    // Test string 1: 12.503+15.43*12-(2m + 5M) >> -4.9998e+06
    // Test string 2: a1=b=d=10*3.1415*10 >> 314.15
    // Test string 3: _def=-1m*a1/10 >> -0.031415
    do
    {
        /* Display ready message and wait for user input */
        if (!silentFlag){
            flog << CONSOLE_READY_MSG;
            flog.flush();
        }
        std::cin.clear();
        std::getline(std::cin, input);
        /* Echo input if the pipe flag is present */
        if (pipeFlag && !silentFlag)
            flog << input << std::endl;
        else
            flog.log(input+"\n");

        /* Skip processing if input is empty */
        if (input.empty()){
            if (!silentFlag)
                flog << "[INFO] Empty input" << std::endl;
            continue;
        } else if (input == "exit")
            break;

        /* Load the input expression into the engine */
        eng.load(input);

        /* Evaluate the loaded expression.
        * Note that multiple expression can be loaded before the eval method is called
        */
        eng.eval();
        print_verbose("[DEBUG] Variable names: "+mbcs::get_printable_vector(eng._varNames));
        print_verbose("[DEBUG] Variable values: "+mbcs::get_printable_vector(eng._varValues));
        /* Check and print any warnings */
        if (!eng.getWarningMsg().empty())
            flog << eng.getWarningMsg();
        /* Check for any errors if there are none print the result to output */
        if (!eng.getErrorMsg().empty())
            flog << eng.getErrorMsg();
        else{
            /* Get the last result */
            while ((result = eng.getResult()) != mbc::RESULT_END)
                result_old = result;
            flog << result_old << std::endl;
        }
    } while (input != "exit");

    /* Perform all cleanup duties before exiting */
    self_cleanup(silentFlag);
    return 0;
}
