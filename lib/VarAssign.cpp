// File
//  VarAssign.cpp
//
// DESCRIPTION:
//  Generates Variable Assignment Logging instrumentation in LLVM IR for
//  insertion into source code at compile time.
//
//  Traverses each instruction and logs loads and stores.
//  Visits all Instructions in each BasicBlock per Func Per Module & adds the
//  above generated instrumentation to the BasicBlocks following assignments
//
// USAGE:
//      opt -load-pass-plugin=libVarAssign.so -passes="varassign"
//          <input_llvm_bc> -o <updated_llvm_bc>
//
// NOTES:
// 
// TODO
//  - Switch to header defined log strings
//=============================================================================
#include "VarAssign.h"
#include "ir_stdio.h"
#include "ir_logger.h"
#include "ir_stdlib.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/Verifier.h"

#include <logger.h>                   // LogLevel_INFO

/*
#include <regex>
#include <iomanip>
#include <iostream>
*/

using namespace llvm;
using namespace varassign;

#define DEBUG 0

GlobalVariable* logFileName;
GlobalVariable* line;                 // TODO Always zero; preproc constraint

//------------------------------------------------------------------------------
// Some of Jay's LLVM support functions
//------------------------------------------------------------------------------
#if DEBUG
/**
 * @brief Dumps via errs() the name and instructions of a basicblock.
 *
 * This function prints a summary of a basicblock in its current state while
 * running an llvm pass.
 *
 * @param BB The basicblock being printed.
 * 
 * @example FuncLog.cpp
 *
 * @usage
 * dumpBB(someBB); // someBB must be a BasicBlock*
 */
void dumpBB(BasicBlock* BB) {
    errs() << "Block: " << BB->getName() << "\n";
    for (auto &I : *BB) {
        errs() << "\t" << I << "\n";
    }
}
#endif

std::string get_value_name(Value* val) {
    if (val->hasName())
        return val->getName().str();
        
    // Handle Unnamed Values by providing the temp name (e.g. %19)
    std::string tempName;
    raw_string_ostream rso(tempName);
    val->printAsOperand(rso, false);
    return rso.str();
}

GlobalVariable* createGlobalInt(Module &M, Type* Ty, const char *c) {
    /*
     * Usage:
     *  GlobalVariable* varName = createGlobalInt(Module, Type, "varNameString");
     */
    return new GlobalVariable(M,
            Ty,
            false,
            GlobalVariable::ExternalLinkage,
            ConstantInt::get(Ty, 0),
            c);
}

//------------------------------------------------------------------------------
// VarAssign Pass Supporting Functions
//------------------------------------------------------------------------------
/**
 * @brief Sets up logging instrumentation to enable tracking execution inline.
 *
 * This function injects a basicblock at the top of the main function to enable
 * logging function and basicblock behaviors of the code during execution. If 
 * the function is already there, no action is taken.
 *
 * @param M The LLVM module required for context and generating code for targets
 * being instrumented.
 *
 * @return Whether or not the instrumentation succeeded.
 *
 * @details
 * TODO Write detailed example of what happens here
 * This function sets up logging by initializing a small file logger and setting
 * the log level. This is setup at the top of main in a block named "setupBlock"
 * that branches directly to the original setup block.
 * 
 * @usage
 * if (!logSetup(M))
 *      // Throw
 */
bool VarAssign::logSetup(Module &M) {
    auto &CTX = M.getContext();

    // Build Functions to add as call instructions
    FunctionCallee getPid = stdlib::getpid(M);
    FunctionCallee snPrintF = ir_stdio::snprintf(M);
    FunctionCallee loggerSetLevel = logger::loggerSetLevel(M);
    FunctionCallee loggerInitFileLogger = logger::loggerInitFileLogger(M);
    
    // Define Types to use
    Type* Int32Ty = Type::getInt32Ty(CTX);
    Type* Int8Ty  = Type::getInt8Ty(CTX);

    // 
    // Fetch first entry block
    ////////////////////////////////////////////////////////////////////////////
    std::string firstFuncName = "main";
    std::string firstBBName = "entry";
    std::string setupBlockName = "setupLogger";
    Function* entryFunc = M.getFunction(firstFuncName);
    BasicBlock* originalBB = &entryFunc->getEntryBlock();
    
    // Check to see if setupLogger BB exists
    if (originalBB->getName().str()) {
        // Populate LogFileName from existing globals
        logFileName = M.getGlobalVariable("logFileName");

#if DEBUG
        errs() << "Logger already setup: " << logFileName << "\n";
#endif 
        return true;
    }
    
    //
    // SETUP LOGGER
    /////////////////////////////////////////////////////////////////////////////
    // Setup Basic Block
#if DEBUG
    dumpBB(originalBB);
#endif
    
    // Set insert point at first basicblock
    IRBuilder bldr(originalBB);
    BasicBlock *entryBB1 = BasicBlock::Create(
            CTX,
            setupBlockName,
            originalBB->getParent()
    );
    entryBB1->moveBefore(originalBB);
    bldr.SetInsertPoint(entryBB1);

    // Populate Globals
    //  line
    //  NOTE IN HINDSIGHT THIS ISN'T REALLY NEEDED if we know we are getting 0
    line = createGlobalInt(M, Int32Ty, "line");
    bldr.CreateStore(bldr.getInt32(0), line);

    //  filename
    //      split on forward-slash
    std::string fullfilename = entryFunc->getParent()->getSourceFileName();
    std::string delimiter = "/";
    size_t pos = fullfilename.find_last_of(delimiter);
    std::string filename = (pos == std::string::npos) ? fullfilename : fullfilename.substr(pos + 1);

    //      split on file extension
    delimiter = ".";
    pos = filename.find_first_of(delimiter);
    filename = (pos == std::string::npos) ? filename : filename.substr(0, pos);
    
    //      create format string
    Constant* tmpcnst = bldr.CreateGlobalStringPtr(filename.append("-%d.log"), "logfilename", 0, &M);

    //  PID
    AllocaInst* pidAlloca = bldr.CreateAlloca(Int32Ty, 0, "pidAlloca");
    Value* pid = bldr.CreateCall(getPid, {}, "getpid");
    bldr.CreateStore(pid, pidAlloca);
    Value* loadPID = bldr.CreateLoad(Int32Ty, pidAlloca);

    // snprintf to generate filename to initialize
    //  snprintf(buffer, size, format, args)
    //      size - 50
    int logFileNameSize = 50;
    ArrayType* lFNArrayTy = ArrayType::get(Int8Ty, logFileNameSize);
    
    //      buffer - Create the global variable for the character array
    //             - Initialize to 0
    Constant* initializer = ConstantAggregateZero::get(lFNArrayTy);
    logFileName = new GlobalVariable(
            M,                              // Module where it should be added
            lFNArrayTy,                     // Array type (char[50])
            false,                          // isConstant
            GlobalValue::ExternalLinkage,   // Linkage type
            initializer,                    // Initializer for the array (zero-filled in this case)
            "logFileName"                   // Name of the global variable
    );
    bldr.CreateCall(snPrintF, {logFileName, bldr.getInt32(logFileNameSize), tmpcnst, loadPID}, "");

    //  Concatinate filename-PID
    //  NOTE if used across files, filename should be generic.
    //       filename could then be added to the log itself as a data source.
    bldr.CreateCall(loggerInitFileLogger, {logFileName, bldr.getInt64(1024*1024), bldr.getInt32(3)}, "");

    // Set Logging Level
    bldr.CreateCall(loggerSetLevel, {bldr.getInt32(LogLevel_INFO)}, "");
    bldr.CreateBr(originalBB);

#if DEBUG
    dumpBB(entryBB1);
#endif

    return true;
}

            // Examine Function Calls
            if (auto *CI = dyn_cast<CallInst>(&I)) {
                std::string cFName = get_func_name(CI).str();

            // Log function assignments by checking to see if stored vals are
            // functions
            else if (auto *CI = dyn_cast<StoreInst>(&I)) {
                Value *storedValue = CI->getValueOperand();
                if (Function *func = dyn_cast<Function>(storedValue)) {
                    logMsg = FuncLog::fAssign + func->getName().str();

                    bldr.SetInsertPoint(&I);
                    Constant *funcAssign = bldr.CreateGlobalStringPtr(logMsg, "FuncAssign", 0, F.getParent());
                    bldr.CreateCall(loggerLog, {bldr.getInt32(LogLevel_INFO), logFileName, bldr.getInt32(0), funcAssign}, "");
                }
            }

/**
 * @brief Insert logging instrumentation above found load instruction
 *
 * This function injects a logging instruction that notifies on value loads
 * as the underlying code executes.
 *
 * @param I The LLVM-IR Instruction that will be logged as a load instruction
 *
 * @return Void
 *
 * @details TODO
 *
 * @usage
 * if (isa<LoadInst>(Instruction* I)
 *      logLoad(I);
 */
void logLoad(Instruction* I) {
    std::string logMsg = VarAssign::loadInst;

    // Get instruction information
    Type* loadIType = I->getType();
    StringRef loadIDest = I->getName();
    StringRef loadISrcAddr = I->getPointerOperand()->getName();

    // Setup logging instruction
    Module* M = I->getModule();
    FunctionCallee loggerLog = logger::loggerLog(*M);

    // Set insertion point above target instruction
    IRBuilder bldr(I->getContext());
    bldr.SetInsertPoint(firstI);
        
    // Insert LoadInst Logging Instruction
    Constant* loadI = bldr.CreateGlobalStringPtr(logMsg, "loadI", 0, M);
    bldr.CreateCall(loggerLog, {bldr.getInt32(LogLevel_INFO), logFileName, bldr.getInt32(0), loadI}, "");


    return;
}

/**
 * @brief Insert logging instrumentation above found store instruction
 *
 * This function injects a logging instruction that notifies on value stores
 * as the underlying code executes.
 *
 * @param I The LLVM-IR Instruction that will be logged as a store instruction
 *
 * @return Void
 *
 * @details TODO
 *
 * @usage
 * if (isa<StoreInst>(Instruction* I)
 *      logStore(I);
 */
void logStore(Instruction* I) {
    std::string logMsg = VarAssign::storeInst;
    return;
}

/**
 * @brief Instruments all basicblocks in all functions available during
 * analysis.
 *
 * This function instruments all variable assignments present in the codebase
 * as they execute.
 *
 * @param M The LLVM Module providing functions and context for the code being
 * instrumented
 *
 * @return Whether or not instrumentation succeeded.
 *
 * @details
 * TODO Write detailed example of what happens here
 * 
 * @usage
 * if (!instrumentAllAssignments(M))
 *      // Throw
 */
bool instrumentAllAssignments(Module &M) {
    // Loop through functions
    for (auto &F : M) {
        // Can't Instrument a declaration
        if(F.isDeclaration())
            continue;
    
        // Loop through BBs in Function
        for (auto &BB : F) {
#if DEBUG
        errs() << "\tTMP Name: " << BB.getName() << "\n";
        errs() << "\tIn BBName: " << bbName << "\n";
        errs() << "\tpre-mod\n";
        dumpBB(&BB);
#endif

#if DEBUG
        errs() << "\tpost-mod\n";
        dumpBB(&BB);
#endif
            // Dodge setupLogger
            if (BB.getName().str() == "setupLogger")
                continue;


            for (auto &I : BB) {
                // Log Load
                if (isa<LoadInst>(&I))
                    logLoad(&I);

                // Log Store
                if (isa<StoreInst>(&I))
                    logStore(&I);
            }

            // TODO Log Phi Nodes
            //  bb1:
            //      br label %bb2
            //
            //  bb2:
            //      %a = phi i32 [ 42, %bb1 ], [ 0, %bb3 ]
            //
            // TODO Log Implicit Assignment
            //  %a = add i32 5, 10       ; %a = 15
            //  %b = mul i32 %a, 2       ; %b = %a * 2 = 30
            // TODO Log Function Arguments
            //  define void @foo(i32 %x) {
            //      ; %x is assigned the argument value
            //      }
            // TODO Log Select Instruction Resolution
            //  %cond = icmp eq i32 %a, %b
            //  %value = select i1 %cond, i32 1, i32 0
            // TODO Log assignments via Memory Intrinsics
            //  (memcpy, memmove, memset)
            //  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dest,
            //                                      i8* %src,
            //                                      i64 10,
            //                                      i1 false)
        }
    }
    return true;
}

//------------------------------------------------------------------------------
// VarAssign Pass Module Code
//------------------------------------------------------------------------------
PreservedAnalyses VarAssign::run(Module &M, ModuleAnalysisManager &) {
    return ( runOnModule(M) ? PreservedAnalyses::none()
           : PreservedAnalyses::all());
}

bool VarAssign::runOnModule(Module &M) {
    // TODO This whole thing can probably be taken out
    // Copy existing basicblocks
    for (auto &F : M) {
        // Populate the vector with the function's current basic blocks
        for (auto &BB : F) {
            originalBlocks[&F].push_back(&BB);
        }
    }

    if (!logSetup(M)) {
        errs() << "Failed to set up logging library instrumentation\n";
        exit(1);
    }

    if (!instrumentAllAssignments(M)) {
        errs() << "Failed to instrument functions\n";
        exit(1);
    }
   
    //
    // VERIFY INSTRUMENTED IR
    ////////////////////////////////////////////////////////////////////////////
    bool Broken = verifyModule(M, &llvm::errs());
    if (Broken) {
        llvm::errs() << "Module verification failed after VarAssign transformations.\n";
        M.dump();
        assert(!Broken && "Verification failed after transformations.");
    }
    // end VERIFY INSTRUMENTED IR //////////////////////////////////////////////
    return true;
}

//------------------------------------------------------------------------------
// New PM Registration
//------------------------------------------------------------------------------
PassPluginLibraryInfo getVarAssignPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "VarAssign", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM,
                        ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "varassign") {
                        MPM.addPass(VarAssign());
                        return true;
                    }
                    return false;
            });
        }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getVarAssignPluginInfo();
}

