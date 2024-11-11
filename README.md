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

### Prerequisites

All pre-requisites for the project are included in the Containerfile and can be installed locally.

### Container Run

If you would like to test, alter, or build the project in an isolated environment, feel free to use our Containerfile. It will work with either docker or podman, and will ensure you get a consistent environment to work with and test the pass.

Because the repo is private, you will need to copy some files into the pass directory so the repo can be fetched to the container:

1. Copy your `.gitconfig` to the local clone from the [Local Installation](#local-installation) step 1.
    ```sh
    cp ~/.gitconfig /path/to/funclog
    ```

    If you don't have one, it should look something like this:
    ```
    [user]
        email = user@email.com
        name = n3ar
    ```

2. Copy the private key you use for GitHub access to the funclog home directory:
    ```sh
    cp ~/.ssh/id_rsa /path/to/funclog
    ```

Now the container will use your github account to pull the code down.

Commands for those unfamiliar can be referenced here:
```
podman image build -t funclog .
podman run -d -p 2222:22 --name funclog localhost/funclog:latest
podman container [start/stop/pause/stats] funclog
```

### Local Installation

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
    # Args are mutually exclusive, sudo may be required.
    ./build.sh [-i]
    ```
    
## Usage

This pass must be used with opt or integrated into an installation of LLVM. If one wanted to use this pass to instrument a file, `hello.c`, this is how one would use it if not installed on path:
```sh
# Emit LLVM-IR (human readable) for hello.c
clang -S -emit-llvm hello.c

# Run opt pass on hello.ll emited from the above to instrument
# built without -i
opt -load-pass-plugin="${APP_HOME}/build/lib/libFuncLog.so" -passes="funclog" -S hello.ll -o instrumented-hello.ll
# built with -i
opt -load-pass-plugin=libFuncLog.so -passes="funclog" -S hello.ll -o instrumented-hello.ll

# Compile the now instrumented LLVM
clang -llogger instrumented-hello.ll -o hello -v

# Run the output binary
./hello
```

## TODO
- DOxygen Documentation
- Proper Test Cases
- CI/CD
