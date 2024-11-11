# FuncLog

This project is an LLVM pass that instruments a program being compiled with function logging notifying on function entry, return, call, and assignment.

This is useful during analysis

## Table of Contents
- [About the Project](#about-the-project)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [TODO](#todo)

## About the Project

### Built With
- [LLVM](https://llvm.org)
- [CMAKE](https://cmake.org)
- [c-logger](https://github.com/N3ar/c-logger)

This project aims to automate instrumentation around function level actions for use with program analysis. This instrumentation allows an analyst to track function behaviors during execution.

## Getting Started

### Testing the project

If you would like to test, alter, or build the project in an isolated environment, feel free to use our Containerfile. It will work with either docker or podman, and will ensure you get a consistent environment to work with and test the pass.

Commands for those unfamiliar can be referenced here:
```
podman image build -t funclog .
podman run -d -p 2222:22 --name funclog localhost/funclog:latest
podman container [start/stop/pause/stats] funclog
```

### Prerequisites

All pre-requisites for the project are included in the Containerfile and can be installed locally.

### Installation

1. Clone the repo
    ```sh
    # USE THE SSH CLONE CUZ PRIVATE
    git clone git@github.com:N3ar/funclog.git
    ```
   
2. Build the pass
    ```sh
    # No args will build the pass locally, with the .so in ${APP_HOME}/build/lib/libFuncLog.so
    #   -i copies .so to /usr/local/lib
    #   -c cleans the build directory
    # Args are mutually exclusive
    ./build.sh [-i]
    ```
    
## Usage

This pass must be used with opt or integrated into an installation of LLVM. If one wanted to use this pass to instrument a file, `hello.c`, this is how one would use it if not installed on path:
```sh
# Emit LLVM-IR (human readable) for hello.c
clang -S -emit-llvm hello.c

# Run opt pass on hello.ll emited from the above to instrument
opt -load-pass-plugin="${APP_HOME}/build/lib/libFuncLog.so" -passes="funclog" -S hello.ll -o instrumented-hello.ll

# Compile the now instrumented LLVM
clang -llogger instrumented-hello.ll -o hello -v

# Run the output binary
./hello
```

If you built with -i or you have placed the shared object on PATH, you may leave out the prefix.


## TODO
- DOxygen Documentation
- Proper Test Cases
- CI/CD
