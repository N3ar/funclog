#ifndef _FUNCLOG_STDIO_H_
#define _FUNCLOG_STDIO_H_

#include "llvm/Transforms/Utils/BuildLibCalls.h"

namespace funclog {
    namespace ir_stdio {
        llvm::FunctionCallee printf(llvm::Module &);
        llvm::FunctionCallee snprintf(llvm::Module &);
    }
}

#endif // _FUNCLOG_STDIO_H_
