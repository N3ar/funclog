#ifndef _FUNCLOG_STDLIB_H_
#define _FUNCLOG_STDLIB_H_

#include "llvm/Transforms/Utils/BuildLibCalls.h"

/**
 * @file ir_stdlib.h
 * @brief Provides FunctionCallee(s) for stdlib library functions used in this
 * pass.
 *
 * stdlib.h is used as the provier for getpid. Funclog utilizes this funciton
 * calls. To support injecting the PID into individual log messages. In the
 * event the source does not include these libraries, manual linkage is needed.
 */

namespace funclog {
    namespace stdlib {
        llvm::FunctionCallee getpid(llvm::Module &);
    }
}
#endif // _FUNCLOG_STDLIB_H_
