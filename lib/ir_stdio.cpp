#include "ir_stdio.h"

#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;
using namespace funclog;

// TODO Look at the call functions and make better

FunctionCallee ir_stdio::printf(Module &M) {
    auto &CTX = M.getContext();
    PointerType* aTy = PointerType::getUnqual(Type::getInt8Ty(CTX));

    Type* retTy = Type::getInt32Ty(CTX);
    FunctionType* fTy = FunctionType::get(retTy, aTy, true);

    return M.getOrInsertFunction("printf", fTy);
}

#if 0
snprintf(str, size, const char *format, ...);
int j = snprintf(buffer, 6, "%s\n", s);
#endif
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
