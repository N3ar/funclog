/*********************************************************************
 * @file  ir_stdio.cpp
 * 
 * @brief Implementations of FunctionCallees supporting stdio funcs
 * used in the analysis passes.
 *********************************************************************/
#include "ir_stdio.h"

#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;
using namespace funclog;

/**
 * @brief Generates a FunctionCallee to inject printf into a target
 *
 * This function defines the function allowing the pass to inject calls to
 * printf whether or not stdio.h is included and linked.
 *
 * @param M The LLVM Module whose context we are defining the function within
 *
 * @return FunctionCallee for a function interface injected into the module
 *
 * @usage
 * FunctionCallee pf = printf(M);
 */
FunctionCallee ir_stdio::printf(Module &M) {
    auto &CTX = M.getContext();
    PointerType* aTy = PointerType::getUnqual(Type::getInt8Ty(CTX));

    Type* retTy = Type::getInt32Ty(CTX);
    FunctionType* fTy = FunctionType::get(retTy, aTy, true);

    return M.getOrInsertFunction("printf", fTy);
}

/**
 * @brief Generates a FunctionCallee to inject snprintf into a target
 *
 * This function defines the function allowing the pass to inject calls to
 * snprintf whether or not stdio.h is included and linked.
 *
 * @param M The LLVM Module whose context we are defining the function within
 *
 * @return FunctionCallee for a function interface injected into the module
 *
 * @usage
 * FunctionCallee snpf = snprintf(M);
 */
FunctionCallee ir_stdio::snprintf(Module &M) {
    auto &CTX = M.getContext();

    Type* retTy = Type::getInt32Ty(CTX);
    std::vector<Type *> args;
    args.push_back(PointerType::getUnqual(Type::getInt8Ty(CTX)));
    args.push_back(Type::getInt32Ty(CTX));
    args.push_back(PointerType::getUnqual(Type::getInt8Ty(CTX)));

    FunctionType* fTy = FunctionType::get(retTy, args, true);

    return M.getOrInsertFunction("snprintf", fTy);
}
