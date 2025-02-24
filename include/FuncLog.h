/**
 * @file FuncLog.h
 *
 * @brief Defines the funclog pass and variables
 *
 * Using the new pass manager interface, define the LLVM Pass implementing
 * Function Call, Entry, Exit, and Assignment LLVM instrumentation.
 */

#ifndef FUNCLOG_H_
#define FUNCLOG_H_

#include "llvm/IR/PassManager.h"

#include <map>
#include <vector>

#define LOGFILE_NAME "funclogfile"

/**
 * The FuncLog struct.
 * This struct defines the LLVM pass by extending PassInfoMixin<Struct Name>
 */
struct FuncLog : public llvm::PassInfoMixin<FuncLog> {
    static const std::string fEntry;        /**< struct string fEntry. Log message prefix.*/
    static const std::string fRet;          /**< struct string fRet. Log message prefix.*/
    static const std::string programExit;   /**< struct string programExit. Log message prefix.*/
    static const std::string programAbort;  /**< struct string programAbort. Log message prefix.*/
    static const std::string fCall;         /**< struct string fCall. Log message prefix.*/
    static const std::string fAssign;       /**< struct string fAssign. Log message prefix.*/
    static const std::string bEntry;        /**< struct string bEntry. Log message prefix.*/

    /**
     * Runs the FungLog LLVM Pass and ensures the effects are preserved.
     * @param Module& The LLVM module currently being run.
     * @param ModuleAnalysisManager& The new pass manager.
     * @return The results of the Funclog Pass
     */
    llvm::PreservedAnalyses run(llvm::Module &, llvm::ModuleAnalysisManager &);
    
    /**
     * Runs the FuncLog Pass on the current module
     * @param Module& The LLVM module being instrumented with FuncLog
     * @return Boolean success results
     */
    bool runOnModule(llvm::Module &);

    /**
     * Injects instrumentation enabling logging before main.
     * @param Module& The LLVM module containing the code being instrumented
     * @return Boolean success results
     */
    bool logSetup(llvm::Module &);

    /**
     * Keyword function asserting that this pass must be run if included
     * @return Bool specifying whether or not it is required
     */
    static bool isRequired() { return true; }
    
    /**
     * A map retaining the original basicblocks
     */
    std::map<llvm::Function*, std::vector<llvm::BasicBlock*>> originalBlocks;
};

// Populated string prefixes
const std::string FuncLog::fEntry       = "Func Entered: ";
const std::string FuncLog::fRet         = "Func Return: ";
const std::string FuncLog::programExit  = "Program Exit: ";
const std::string FuncLog::programAbort = "Program Abort: ";
const std::string FuncLog::fCall        = "Func Call: ";
const std::string FuncLog::fAssign      = "Func Assignment: ";
const std::string FuncLog::bEntry       = "BasicBlock Entry: ";


#endif // FUNCLOG_H_
