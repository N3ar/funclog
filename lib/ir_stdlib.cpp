// File
//  ir_stdlib.cpp
//
// DESCRIPTION:
//  Small class that generates funcitons in LLVM IR for insertion into targets.
//
//  This should make instrumentation a bit easier.
//
// USAGE:
//  Calling these functions should leave us with an instruction that we can
//  immediately use in a create call with builder.
//
//==============================================================================
#include "ir_stdlib.h"

using namespace llvm;

using namespace funclog;
FunctionCallee stdlib::getpid(Module &M) {
    // args: void
    // ret:  pid_t (uint32_t)
    FunctionType *FTy = FunctionType::get(Type::getInt32Ty(M.getContext()), false);
    return M.getOrInsertFunction("getpid", FTy);
}

