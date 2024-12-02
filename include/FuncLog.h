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
    static const std::string fEntry; 
    static const std::string fRet;
    static const std::string programExit;
    static const std::string programAbort;
    static const std::string fCall;
    static const std::string fAssign;
    static const std::string bEntry;

    llvm::PreservedAnalyses run(llvm::Module &, llvm::ModuleAnalysisManager &);
    bool runOnModule(llvm::Module &);
    bool logSetup(llvm::Module &);

    static bool isRequired() { return true; }
    std::map<llvm::Function*, std::vector<llvm::BasicBlock*>> originalBlocks;
};

const std::string FuncLog::fEntry       = "Func Entered: ";
const std::string FuncLog::fRet         = "Func Return: ";
const std::string FuncLog::programExit  = "Program Exit: ";
const std::string FuncLog::programAbort = "Program Abort: ";
const std::string FuncLog::fCall        = "Func Call: ";
const std::string FuncLog::fAssign      = "Func Assignment: ";
const std::string FuncLog::bEntry       = "BasicBlock Entry: ";


#endif // FUNCLOG_H_
