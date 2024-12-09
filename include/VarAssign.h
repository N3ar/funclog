//==============================================================================
//  FILE:
//      VarAssign.h
//
//  DESCRIPTION:
//      Using the new pass manager interface, define the LLVM Pass implementing
//      Variable Store and Load LLVM instrumentation.
//
//==============================================================================
#ifndef VARASSIGN_H_
#define VARASSIGN_H_

#include "llvm/IR/PassManager.h"

#include <map>
#include <vector>

#define LOGFILE_NAME "funclogfile"

struct VarAssign : public llvm::PassInfoMixin<VarAssign> {
    static const std::string loadI; 
    static const std::string storeI;

    llvm::PreservedAnalyses run(llvm::Module &, llvm::ModuleAnalysisManager &);
    bool runOnModule(llvm::Module &);
    bool logSetup(llvm::Module &);

    static bool isRequired() { return true; }
    std::map<llvm::Function*, std::vector<llvm::BasicBlock*>> originalBlocks;
};

const std::string VarAssign::loadI      = "Load ";
const std::string VarAssign::storeI     = "Store ";

#endif // VARASSIGN_H_
