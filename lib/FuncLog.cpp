// File
//  FuncLog.cpp
//
// DESCRIPTION:
//  Generates Function Logging instrumentation in LLVM IR for insertion into
//  source code.
//
//  Visits each function and instruments function entry, exit, and call.
//  Calling instrumentation includes function pointers and assignment.
//  Visits all the BasicBlocks per Func Per Module & adds the above generated
//  instrumentation to the BasicBlocks
//
// USAGE:
//      opt -load-pass-plugin=libGneiss.so -passes="gneiss"
//          <input_llvm_bc> -o <updated_llvm_bc>
//
// NOTES:
//  Currently I have logBBEntry as a funciton outside of the function that
//  traverses all basicblocks. This means that I am running the same loop again
//  It is inefficient but clear.
// 
// TODO
//  - Details in documentation
//  - logFuncRet - Check for no-return functions
//  - Consider logBBEntry inside of logFuncCall to minimize looping
//  - Switch to header defined log strings
//=============================================================================
#include "FuncLog.h"
#include "ir_stdio.h"
#include "ir_logger.h"
#include "ir_stdlib.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/Verifier.h"

#include <logger.h>                   // LogLevel_INFO

#include <regex>
#include <iomanip>
#include <iostream>

using namespace llvm;
using namespace funclog;

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

/**
 * @brief Gets the name of a function referred in a call instruction.
 *
 * This function gets the name of a function being called in a CallInst as a
 * StringRef during an LLVM Compiler pass.
 *
 * @param call The CallInstruction (or correctly dynamically cast Instruction)
 * to fetch the call targets name from.
 *
 * @return The name of the function as a StringRef
 *
 * @details
 * The name of the function is usually a string. In the event that this is an
 * indirect call, the words "indirect call" are returned instead.
 * 
 * @usage
 * StringRef strRef = get_func_name(callInst);
 * std::string str = get_func_name(callInst).str();
 * StringRef funcName = get_func_name(dyn_cast<CallInst>(call));
 */
StringRef get_func_name(CallInst* call) {
    Function* f = call->getCalledFunction();
    if (f)
        return f->getName();
    else
        return StringRef("Indirect Call");
}

std::string get_value_name(Value* val) {
    if (val->hasName())
        return val->getName().str();
        
    // Handle Unnamed Values by providing the temp name (e.g. %19)
    std::string tempName;
    raw_string_ostream rso(tempName);
    val->printAsOperand(rso, false);
    return rso.str();
}

bool is_exit_call(Instruction* call) {
    // Ensure instruction is a call instruction.
    if (!isa<CallInst>(call))
        return false;

    // Compare function call name with "exit"
    std::string tgtFunc = "exit";
    StringRef funcName = get_func_name(dyn_cast<CallInst>(call));
    return funcName.equals(tgtFunc);
}

bool is_abort_call(Instruction* call) {
    // Ensure instruction is a call instruction.
    if (!isa<CallInst>(call))
        return false;

    // Compare function call name with "exit"
    std::string tgtFunc = "abort";
    StringRef funcName = get_func_name(dyn_cast<CallInst>(call));
    return funcName.equals(tgtFunc);
}


bool is_main_ret(Instruction* i) {
    std::string tgtName = "main";
    Function* f = i->getFunction();
    return (!f->getName().compare(tgtName) && isa<ReturnInst>(i));
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
// FuncLog Pass Supporting Functions
//------------------------------------------------------------------------------
/**
 * @brief Sets up logging instrumentation to enable tracking execution inline.
 *
 * This function injects a basicblock at the top of the main function to enable
 * logging function and basicblock behaviors of the code during execution.
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
bool FuncLog::logSetup(Module &M) {
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
    Function* entryFunc = M.getFunction(firstFuncName);
    BasicBlock* originalBB = &entryFunc->getEntryBlock();
    
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
            "setupLogger",
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

#if DEBUG
    // Logger Debug Statement
    FunctionCallee loggerLog = logger::loggerLog(M);
    std::string localTest = "Test String Achieved!";
    Constant* temptest = bldr.CreateGlobalStringPtr(localTest, "test log", 0, &M);
    bldr.CreateCall(loggerLog, {bldr.getInt32(LogLevel_INFO), logFileName, bldr.getInt32(0), temptest}, "");
#endif
    bldr.CreateBr(originalBB);

#if DEBUG
    dumpBB(entryBB1);
#endif

    return true;
}

/**
 * @brief Logs all function entry events.
 *
 * This function injects a logging instruction at the start of the first
 * basicblock of a function to register its entry.
 *
 * @param F The function being instrumented
 *
 * @return void
 *
 * @details
 * TODO Write detailed example of what happens here
 * 
 * @usage
 * logFuncEntry(F);
 */
void logFuncEntry(Function &F) {
    std::string funcName = F.getName().str();
    std::string logMsg = FuncLog::fEntry + funcName;
    
    Module* M = F.getParent();
    FunctionCallee loggerLog = logger::loggerLog(*M);

    // Get Entry BB
    BasicBlock* BB = &F.getEntryBlock();
    if(funcName == "main")
        BB = BB->getSingleSuccessor();
        
#if DEBUG
    errs() << "In Func: " << funcName << "\n";
    errs() << "\tpre-mod\n";
    dumpBB(BB);
#endif

    // Set insert point at top of the BB
    IRBuilder bldr(BB);

    //  Get top instruction of BB
    Instruction* firstI = BB->getFirstNonPHI();
    bldr.SetInsertPoint(firstI);
        
    // Insert Entry Logging Instruction
    Constant* funcEntry = bldr.CreateGlobalStringPtr(logMsg, "FuncEntry", 0, F.getParent());
    bldr.CreateCall(loggerLog, {bldr.getInt32(LogLevel_INFO), logFileName, bldr.getInt32(0), funcEntry}, "");

#if DEBUG
    errs() << "\tpost-mod\n";
    dumpBB(BB);
#endif

    return;
}

/**
 * @brief Logs returns from functions.
 *
 * This function injects a logging instruction at each return instruction
 * within a function.
 *
 * @param F The function being instrumented
 *
 * @return void
 *
 * @details
 * TODO Write detailed example of what happens here
 * 
 * @usage
 * logFuncRet(F);
 */
void logFuncRet(Function &F) {
    std::string funcName = F.getName().str();
   
    Module* M = F.getParent();
    FunctionCallee loggerLog = logger::loggerLog(*M);

    // Check for return instructions (exit & abort are in calls)
    IRBuilder bldr(F.getContext());
    for (auto &BB : F) {
        std::string logMsg;
        Instruction* I = BB.getTerminator();

        if (!isa<ReturnInst>(I)) {
            continue;
        }

        logMsg = FuncLog::fRet + funcName;

        // If the logMessage has been populated, insert.
        if (!logMsg.empty()) {
            bldr.SetInsertPoint(I);
            Constant* funcExit = bldr.CreateGlobalStringPtr(logMsg, "FuncExit", 0, F.getParent());
            bldr.CreateCall(loggerLog, {bldr.getInt32(LogLevel_INFO), logFileName, bldr.getInt32(0), funcExit}, "");
        }
    }
    return;
}

/**
 * @brief Logs function calls within a function.
 *
 * This function injects a logging instruction at each call instruction
 * within a function.
 *
 * @param F The function being instrumented
 *
 * @return void
 *
 * @details
 * TODO Write detailed example of what happens here
 * 
 * @usage
 * logFuncCall(F);
 */
void logFuncCall(Function &F) {
    Module *M = F.getParent();
    FunctionCallee loggerLog = logger::loggerLog(*M);

    std::string logMsg;
    std::string funcName = F.getName().str();

    IRBuilder bldr(F.getContext());
    for (auto &BB : F) {
        // Dodge setupLogger BB
        if (BB.getName().str() == "setupLogger")
            continue;

        // Check each instruction for calls
        for (auto &I : BB) {

            // Examine Function Calls
            if (auto *CI = dyn_cast<CallInst>(&I)) {
                std::string cFName = get_func_name(CI).str();

                if (is_exit_call(CI))
                    logMsg = FuncLog::programExit + funcName;
                else if (is_abort_call(CI))
                    logMsg = FuncLog::programAbort + funcName;
                else
                    logMsg = FuncLog::fCall + cFName;

                // Handle indirect calls
                if (cFName == "Indirect Call")
                    // Do NOT forget the escape character %
                    logMsg += " to -> %" + get_value_name(CI->getCalledOperand());

                // Generate log instruction
                bldr.SetInsertPoint(&I);
                Constant* funcCall = bldr.CreateGlobalStringPtr(logMsg, "FuncCall", 0, F.getParent());
                bldr.CreateCall(loggerLog, {bldr.getInt32(LogLevel_INFO), logFileName, bldr.getInt32(0), funcCall}, "");
            }

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
        }
    }

    return;
}

/**
 * @brief Logs basicblock entries within a function.
 *
 * This function injects a logging instruction at the top of each basicblock
 * within a function.
 *
 * @param F The function being instrumented
 *
 * @return void
 *
 * @details
 * TODO Write detailed example of what happens here
 * 
 * @usage
 * logBBEntry(F);
 */
void logBBEntry(Function &F) {
    Module *M = F.getParent();
    FunctionCallee loggerLog = logger::loggerLog(*M);

    std::string funcName = F.getName().str();
    std::string logMsg;

    uint32_t bbNum = 0;
    IRBuilder bldr(F.getContext());
    for (auto &BB : F) {
        // Dodge setup logger
        std::string bbName = BB.getName().str();
        if (bbName == "setupLogger")
            continue;

        // Handle Empty BasicBlock Names
        if (!BB.hasName()) {
            // Generate BBName
            std::ostringstream oss;
            oss << funcName << "-" << std::setfill('0') << std::setw(2) << bbNum;
            bbName = oss.str();

            // Set BBName to BasicBlock
            BB.setName(bbName);
        }

        // Generate log message
        logMsg = FuncLog::bEntry + bbName;

#if DEBUG
        errs() << "\tTMP Name: " << BB.getName() << "\n";
        errs() << "\tIn BBName: " << bbName << "\n";
        errs() << "\tpre-mod\n";
        dumpBB(&BB);
#endif

        // Set insert point at top of the BB
        IRBuilder bldr(&BB);

        //  Get top instruction of BB
        Instruction* firstI = BB.getFirstNonPHI();
        bldr.SetInsertPoint(firstI);
        
        // Insert Entry Logging Instruction
        Constant* bbEntry = bldr.CreateGlobalStringPtr(logMsg, "BBEntry", 0, F.getParent());
        bldr.CreateCall(loggerLog, {bldr.getInt32(LogLevel_INFO), logFileName, bldr.getInt32(0), bbEntry}, "");

        // Increment BB Counter
        ++bbNum;
#if DEBUG
        errs() << "\tpost-mod\n";
        dumpBB(&BB);
#endif
    }
}

/**
 * @brief Instruments all functions with this passes analysis.
 *
 * This function instruments all functions and basicblocks in the codebase
 * being instrumented.
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
 * if (!instrumentAllFuncs(M))
 *      // Throw
 */
bool instrumentAllFuncs(Module &M) {
    // Loop through functions
    for (auto &F : M) {
        // Can't Instrument a declaration
        if(F.isDeclaration())
            continue;
    
        logFuncCall(F);
        logBBEntry(F);
        logFuncEntry(F);
        logFuncRet(F);
    }
    return true;
}

//------------------------------------------------------------------------------
// FuncLog Pass Module Code
//------------------------------------------------------------------------------
PreservedAnalyses FuncLog::run(Module &M, ModuleAnalysisManager &) {
    return ( runOnModule(M) ? PreservedAnalyses::none()
           : PreservedAnalyses::all());
}

// TODO Can likely be faster if I just do everything in one loop.
bool FuncLog::runOnModule(Module &M) {
    // TODO This whole thing can probably be taken out
    // Copy existbeingasic blocks
    for (auto &F : M) {
        // Populate the vector with the function's current basic blocks
        for (auto &BB : F) {
            originalBlocks[&F].push_back(&BB);
        }
    }

    // TODO Check to see if logSetup needs to be run or not
    if (!logSetup(M)) {
        errs() << "Failed to set up logging library instrumentation\n";
        exit(1);
    }

    if (!instrumentAllFuncs(M)) {
        errs() << "Failed to instrument functions\n";
        exit(1);
    }
   
    //
    // VERIFY INSTRUMENTED IR
    ////////////////////////////////////////////////////////////////////////////
    bool Broken = verifyModule(M, &llvm::errs());
    if (Broken) {
        llvm::errs() << "Module verification failed after Gneiss transformations.\n";
        M.dump();
        assert(!Broken && "Verification failed after transformations.");
    }
    // end VERIFY INSTRUMENTED IR //////////////////////////////////////////////
    return true;
}

//------------------------------------------------------------------------------
// New PM Registration
//------------------------------------------------------------------------------
PassPluginLibraryInfo getFuncLogPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "FuncLog", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM,
                        ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "funclog") {
                    MPM.addPass(FuncLog());
                    return true;
                    }
                    return false;
                    });
        }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getFuncLogPluginInfo();
}

