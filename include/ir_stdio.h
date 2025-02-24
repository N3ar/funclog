#ifndef _FUNCLOG_STDIO_H_
#define _FUNCLOG_STDIO_H_

#include "llvm/Transforms/Utils/BuildLibCalls.h"

/**
 * @file ir_stdio.h
 * @brief Provides FunctionCallee(s) for stdio library functions used in this
 * pass.
 *
 * stdio.h is used as the provier for printf and snprintf. Funclog utilizes
 * these function calls. To support injecting these into source that may not 
 * include these libraries and thus requires manual linkage.
 */

namespace funclog {
    namespace ir_stdio {
        llvm::FunctionCallee printf(llvm::Module &);
        llvm::FunctionCallee snprintf(llvm::Module &);
    }
}

#endif // _FUNCLOG_STDIO_H_
