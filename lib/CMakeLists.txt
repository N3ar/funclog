# I may have largely copied this from the LLVM tutorial I learned from
#=============================================================================
cmake_minimum_required(VERSION 3.26.0)
project(FuncLog)

#=============================================================================
# 1. LOAD LLVM CONFIGS
#=============================================================================
# Set this to a valid LLVM installation dir
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM Installation Directory")

# Add the location of LLVMConfig.cmake to CMake search paths
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

# FIXME I am not sure if I need to do anything with this or not
find_package(LLVM REQUIRED CONFIG)

# Sanity check with VERSION_GREATER_EQUAL
if(NOT "${LLVM_VERSION_MAJOR}" VERSION_GREATER_EQUAL "18")
    message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 18 or higher")
endif()

# Gneiss includes headers from LLVM
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

#=============================================================================
# 2. GNEISS BUILD CONFIGURATION
#=============================================================================
# Use the same C++ standard as LLVM
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

# LLVM is normally built without RTTI
if(NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

#=============================================================================
# 3. ADD IN TARGETS
#=============================================================================
list(APPEND EXTRA_INCLUDES "../include")

add_library(ir_logger STATIC ir_logger.cpp)
list(APPEND EXTRA_LIBS ir_logger)
target_include_directories(ir_logger PUBLIC ${EXTRA_INCLUDES})

add_library(ir_stdlib STATIC ir_stdlib.cpp)
list(APPEND EXTRA_LIBS ir_stdlib)
target_include_directories(ir_stdlib PUBLIC ${EXTRA_INCLUDES})

add_library(ir_stdio STATIC ir_stdio.cpp)
list(APPEND EXTRA_LIBS ir_stdio)
target_include_directories(ir_stdio PUBLIC ${EXTRA_INCLUDES})

#add_library(ir_unistd STATIC ir_unistd.cpp)
#list(APPEND EXTRA_LIBS ir_unistd)
#target_include_directories(ir_unistd PUBLIC ${EXTRA_INCLUDES})

#add_library(ir_fcntl STATIC ir_fcntl.cpp)
#list(APPEND EXTRA_LIBS ir_fcntl)
#target_include_directories(ir_fcntl PUBLIC ${EXTRA_INCLUDES})

#add_library(ir_err STATIC ir_err.cpp)
#list(APPEND EXTRA_LIBS ir_err)
#target_include_directories(ir_err PUBLIC ${EXTRA_INCLUDES})

add_library(FuncLog SHARED FuncLog.cpp)
target_include_directories(FuncLog PUBLIC
    "${PROJECT_BINARY_DIR}"
    ${EXTRA_INCLUDES}
    )
target_link_libraries(FuncLog PUBLIC ${EXTRA_LIBS})

add_library(VarAssign SHARED VarAssign.cpp)
target_include_directories(VarAssign PUBLIC
    "${PROJECT_BINARY_DIR}"
    ${EXTRA_INCLUDES}
    )
target_link_libraries(VarAssign PUBLIC ${EXTRA_LIBS})

# Allow undefined symbols in shared objects on Darwin
#target_link_libraries(Gneiss 
#    "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
