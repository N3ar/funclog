project(FuncLog)

set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM Installation Directory")
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

find_package(LLVM REQUIRED CONFIG)

# Sanity check with VERSION_GREATER_EQUAL
if(NOT "${LLVM_VERSION_MAJOR}" VERSION_GREATER_EQUAL "18")
    message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 18 or higher")
endif()

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
if(NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

list(APPEND EXTRA_INCLUDES "../include")
list(APPEND EXTRA_LIBS
    ir_stdlib
    ir_logger
    logger
    ir_stdio
    #ir_unistd
    #ir_fcntl
    )

set(CMAKE_C_COMPILER clang)
set(CMAKE_LINKER_FLAGS ${CMAKE_LINKER_FLAGS} "-flto")

add_executable(test-logger test-logger.c)
target_link_libraries(test-logger PUBLIC ${EXTRA_LIBS})
target_include_directories(test-logger PUBLIC
    "${PROJECT_BINARY_DIR}"
    ${EXTRA_INCLUDES}
    )

add_executable(hello hello.c)
target_link_libraries(hello PUBLIC ${EXTRA_LIBS})
target_include_directories(hello PUBLIC
    "${PROJECT_BINARY_DIR}"
    ${EXTRA_INCLUDES}
    )

#add_executable(testFIFO test_FIFO.c)
#target_link_libraries(testFIFO PUBLIC ${EXTRA_LIBS})
#target_include_directories(testFIFO PUBLIC
#    "${PROJECT_BINARY_DIR}"
#    )

#add_executable(hello2 hello2.c)
#add_executable(test test.c)
#add_executable(testPFM test_pfm.c)
#
#target_compile_options(hello2 PUBLIC ${CMAKE_C_FLAGS} ${CMAKE_LINKER_FLAGS})
#target_compile_options(test PUBLIC ${CMAKE_C_FLAGS} ${CMAKE_LINKER_FLAGS})
#target_compile_options(testFIFO PUBLIC ${CMAKE_C_FLAGS} ${CMAKE_LINKER_FLAGS})
#target_compile_options(testPFM PUBLIC ${CMAKE_C_FLAGS} ${CMAKE_LINKER_FLAGS})

# FIXME Need to do some other things as well. We need to add linking for certain
# targets... I just don't know which ones, yet.
#add_library(TestUnistd SHARED test_unistd.cpp)
#target_include_directories(TestUnistd PUBLIC
#    "${PROJECT_BINARY_DIR}"
#    ${EXTRA_INCLUDES}
#    )
#target_link_libraries(TestUnistd PUBLIC ${EXTRA_LIBS})
