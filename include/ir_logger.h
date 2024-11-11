#ifndef _FUNCLOG_LIB_LOGGER_H_
#define _FUNCLOG_LIB_LOGGER_H_

#include "llvm/Transforms/Utils/BuildLibCalls.h"

namespace funclog {
    namespace logger {
        llvm::FunctionCallee loggerInitFileLogger(llvm::Module &);
        llvm::FunctionCallee loggerSetLevel(llvm::Module &);
        llvm::FunctionCallee loggerLog(llvm::Module &);
    }
}

#endif // _FUNCLOG_LIB_LOGGER_H_
