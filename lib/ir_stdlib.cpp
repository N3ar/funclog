/**********************************************************************
 * @file  ir_stdio.cpp
 * 
 * @brief Implementations of FunctionCallees supporting stdio funcs
 * used in the analysis passes.
 *
 * Small class that generates funcitons in llvm ir for insertion into targets.
 * This should make instrumentation a bit easier.
 *********************************************************************/
#include "ir_stdlib.h"

using namespace llvm;
using namespace funclog;

/**
 * @brief Generates a FunctionCallee to inject getpid into a target
 *
 * This function defines the function allowing the pass to inject calls to
 * printf whether or not stdlib.h is included and linked.
 *
 * @param M The LLVM Module whose context we are defining the function within
 *
 * @return FunctionCallee for a function interface injected into the module
 *
 * @usage
 * FunctionCallee gp = getpid(M);
 */
FunctionCallee stdlib::getpid(Module &M) {
    // args: void
    // ret:  pid_t (uint32_t)
    FunctionType *FTy = FunctionType::get(Type::getInt32Ty(M.getContext()), false);
    return M.getOrInsertFunction("getpid", FTy);
}

