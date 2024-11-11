//==============================================================================
//  FILE:
//      FuncLog.h
//
//  DESCRIPTION:
//      Using the new pass manager interface, define the LLVM Pass implementing
//      Function Call, Entry, Exit, and Assignment LLVM instrumentation.
//
//==============================================================================
#ifndef FUNCLOG_H_
#define FUNCLOG_H_

#include "llvm/IR/PassManager.h"

#include <map>
#include <vector>

#define LOGFILE_NAME "funclogfile"

struct FuncLog : public llvm::PassInfoMixin<FuncLog> {
    std::string funcEntry  = "Entering Function: %s";
    std::string funcExit   = "Exiting Function: %s";
    std::string funcCall   = "Calling Function: %s";
    std::string funcAssign = "Assigning Function: %s";

    llvm::PreservedAnalyses run(llvm::Module &, llvm::ModuleAnalysisManager &);
    bool runOnModule(llvm::Module &);
    bool logSetup(llvm::Module &);

    static bool isRequired() { return true; }
    std::map<llvm::Function*, std::vector<llvm::BasicBlock*>> originalBlocks;
};

#endif // FUNCLOG_H_
