//==============================================================================
//  FILE
//      llvm_stdlib.h
//
//  DESCRIPTION
//      
//  USAGE
//      
//
//==============================================================================


#ifndef _FUNCLOG_STDLIB_H_
#define _FUNCLOG_STDLIB_H_

#include "llvm/Transforms/Utils/BuildLibCalls.h"

namespace funclog {
    namespace stdlib {
        llvm::FunctionCallee getpid(llvm::Module &);
    }
}
#endif // _FUNCLOG_STDLIB_H_
