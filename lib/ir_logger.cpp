/*********************************************************************
 * @file  ir_logger.cpp
 * 
 * @brief Implementations of FunctionCallees supporting the logging
 * library key to the analysis passes.
 *********************************************************************/
#include "ir_logger.h"

using namespace llvm;
using namespace funclog;

/**
 * @brief Generates a FunctionCallee to init the file logger
 *
 * This function defines the function initializing the file logger that can be
 * injected into code being compiled during an LLVM pass.
 *
 * @param M The LLVM Module whose context we are defining the function within
 *
 * @return FunctionCallee for a function interface injected into the module
 *
 * @details
 * TODO Write detailed example of what happens here
 * 
 * @usage
 * FunctionCallee lIFL = loggerInitFileLogger(M);
 */
FunctionCallee logger::loggerInitFileLogger(Module &M) {
    // args: (str)filename, (long)filesize, (int)numbackupfiles
    // ret:  (int)
    auto &CTX = M.getContext();
    
    Type* retTy = Type::getInt32Ty(CTX);

    std::vector<Type *> args;
    args.push_back(PointerType::getUnqual(Type::getInt8Ty(CTX)));
    args.push_back(Type::getInt64Ty(CTX));
    args.push_back(Type::getInt32Ty(CTX));
    
    FunctionType* FTy = FunctionType::get(retTy, args, false);
    return M.getOrInsertFunction("logger_initFileLogger", FTy);
}

/**
 * @brief Generates a FunctionCallee to set log level for the logger
 *
 * This function defines the function that sets the log level for injected log
 * instructions
 *
 * @param M The LLVM Module whose context we are defining the function within
 *
 * @return FunctionCallee for a function interface injected into the module
 *
 * @details
 * TODO Write detailed example of what happens here
 * 
 * @usage
 * FunctionCallee lSL = loggerSetLevel(M);
 */
FunctionCallee logger::loggerSetLevel(Module &M) {
    // args: LogLevel level
    // ret:  void
    auto &CTX = M.getContext();

    Type* retTy = Type::getVoidTy(CTX);

    Type* aTy = Type::getInt32Ty(CTX);
    FunctionType *FTy = FunctionType::get(retTy, aTy, false);

    return M.getOrInsertFunction("logger_setLevel", FTy);
}

/**
 * @brief Generates a FunctionCallee to write to the logfile
 *
 * This function defines the function initializing the file logger that can be
 * injected into code being compiled during an LLVM pass.
 *
 * @param M The LLVM Module whose context we are defining the function within
 *
 * @return FunctionCallee for a function interface injected into the module
 *
 * @details
 * TODO Write detailed example of what happens here
 * 
 * @usage
 * FunctionCallee lL = loggerLog(M);
 */
FunctionCallee logger::loggerLog(Module &M) {
    // args: i32(loglevel), ptr(filename), i32(line), ptr(fmt), // VA_ARGS
    // ret:  void
    auto &CTX = M.getContext();

    Type* retTy = Type::getVoidTy(CTX);

    std::vector<Type *> args;
    args.push_back(Type::getInt32Ty(CTX));
    args.push_back(PointerType::getUnqual(Type::getInt8Ty(CTX)));
    args.push_back(Type::getInt32Ty(CTX));
    args.push_back(PointerType::getUnqual(Type::getInt8Ty(CTX)));

    FunctionType *FTy = FunctionType::get(retTy, args, true);

    return M.getOrInsertFunction("logger_log", FTy);
}


