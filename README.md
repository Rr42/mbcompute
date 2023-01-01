# MB compute project 
![Test build](https://github.com/Rr42/mbcompute/actions/workflows/c-cpp.yml/badge.svg)
![CodeQL](https://github.com/Rr42/mbcompute/actions/workflows/codeql.yml/badge.svg)

This is a math compute library and accompanying application that provides a console interface.

## List of supported functions
* Solve basic mathematical expressions

# Building project from scratch
## Installing requirements
### Configuration utilities
To generate the configure script the `autoconf` package will be needed, run the below command to install the package.
```Bash
sudo apt install autoconf
```
> These commands are applicable for Debian based systems

### Build utilities
#### For a Linux target
To install the `g++` utilities required to build this project run the below command.
```Bash
sudo apt install build-essential
```
> These commands are applicable for Debian based systems

#### For a Windows target
To install the `x86_64-w64-mingw32-g++-win32` utilities required to build this project run the below command.
```Bash
sudo apt install mingw-w64
```
> These commands are applicable for Debian based systems

## Building the project
### For a Linux target
The library and console application in this project can be built for Linux (x64) using default GNU complier by running the below command in the top project directory.
```Bash
make TARGETOS=LINUX all
```

### For a Windows x64 target
The library and console application in this project can be built for Windows (x64) using MINGW64's win32 complier by running the below command in the top project directory.
```Bash
make TARGETOS=WIN64 all
```

### For a Windows x86 target
The library and console application in this project can be built for Windows (x86) using MINGW64's win32 complier by running the below command in the top project directory.
```Bash
make TARGETOS=WIN32 all
```

## Cleanup
The project can be cleaned by running `make clean` in the top project directory to clean all build files.

### For a Linux target
To clean files specifically from the Linux (x64) build run the below command.
```Bash
make TARGETOS=LINUX clean
```

### For a Windows x64 target
To clean files specifically from the Windows (x64) build run the below command.
```Bash
make TARGETOS=WIN64 clean
```

### For a Windows x86 target
To clean files specifically from the Windows (x86) build run the below command.
```Bash
make TARGETOS=WIN32 clean
```
