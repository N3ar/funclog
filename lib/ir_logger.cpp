#include "ir_logger.h"

using namespace llvm;
using namespace funclog;

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

FunctionCallee logger::loggerSetLevel(Module &M) {
    // args: LogLevel level
    // ret:  void
    auto &CTX = M.getContext();

    Type* retTy = Type::getVoidTy(CTX);

    Type* aTy = Type::getInt32Ty(CTX);
    FunctionType *FTy = FunctionType::get(retTy, aTy, false);

    return M.getOrInsertFunction("logger_setLevel", FTy);
}

// manage __FILENAME__             F.getParent()->getSourceFileName();
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


