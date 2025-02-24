/**
 * @file VarAssign.h
 *
 * @brief Defines the VarAssign pass and variables
 *
 * Using the new pass manager interface, define the LLVM Pass implementing
 * logging lines on individual variable assignment instructions in target code.
 */

#ifndef VARASSIGN_H_
#define VARASSIGN_H_

#include "llvm/IR/PassManager.h"

#include <map>
#include <vector>

#define LOGFILE_NAME "funclogfile"

/**
 * The VarAssign struct.
 * This struct defines the LLVM pass by extending PassInfoMixin<Struct Name>
 */
struct VarAssign : public llvm::PassInfoMixin<VarAssign> {
    static const std::string loadI;     /** struct string loadI. Log message prefix for variable loads.*/
    static const std::string storeI;    /** struct string storeI. Log message prefix for variable stores.*/

    /**
     * Runs the VarAssign LLVM Pass and ensures the effects are preserved.
     * @param Module& The LLVM module currently being run.
     * @param ModuleAnalysisManager& The new pass manager.
     * @return The results of the VarAssign Pass
     */
    llvm::PreservedAnalyses run(llvm::Module &, llvm::ModuleAnalysisManager &);
    
    /**
     * Runs the VarAssign Pass on the current module
     * @param Module& The LLVM module being instrumented with VarAssign
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

const std::string VarAssign::loadI      = "Load ";
const std::string VarAssign::storeI     = "Store ";

#endif // VARASSIGN_H_
