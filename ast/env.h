#ifndef _MIYUKI_ENV_H
#define _MIYUKI_ENV_H

#include <map>
#include <vector>
#include "ast/declaration.h"
#include "ast/type.h"
#include "parse/ifparser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"

namespace Miyuki::AST {
    
    using namespace std;
    using namespace llvm;

    //typedef Value  Identifier;
    //typedef Value* IdentifierPtr;

    typedef map<string, TypePtr>   TypedefMap;
    typedef map<string, IdentifierPtr> IdentifierMap;

    DEFINE_SHARED_PTR(Scope);

    class Scope {
    public:
        IdentifierMap vars;
    };

    class GlobalScope {
    public:
        /////////// Parsing-time Information ///////
        // these information generates and be used during compile-time

        // alias names by typedef
        //   I put alias type names in 2 places - both type info and typedef names
        TypedefMap    typedefs;

        // type list of all types
        TypeMapPtr    types;

        // intermediate-time dependence scopes
        ScopePtr      scopes = nullptr;
         
        // LLVM Gode Generation Related
        LLVMContext   context;
        BasicBlock*   currentBasicBlock;
        Function*     currentFunction;
        Function*     globalInitFunction;
        IRBuilder<>   Builder;

        // Type imformation
        map<Type*, PackedTypeInformationPtr> DetailedTypeInfo;

        Parse::IParserPtr parser;
    public:
        // instance
        static GlobalScope instance;
        static GlobalScope& getInstance() { return instance; }

    public:
        GlobalScope(): Builder(context) {}
    };

    /// Global Functions
    // *** LLVM Related **
    // create new basic block and set as cuurent insert Basic Block
    BasicBlock* switchBasicBlock(string name);
    // set as current insert Basic Block
    void switchBasicBlock(BasicBlock* BB);
    // LLVM Global Context
    LLVMContext& getGlobalContext();
    // get current insert Basic Block
    BasicBlock* getCurrentBasicBlock();
    // get Current Insert Function
    Function* getCurrentFunction();
    // IRBuilder instance
    extern IRBuilder<>& Builder;
    // default platform pointer size
    extern const size_t PointerSize;
    // LLVM Module Instance
    static Module * TheModule;
    
    // ** Compile-Time Information **
    // get identifier by name
    IdentifierPtr getIdentifier(string name);
    // insert identifier to symbol table
    void addIndentifier(IdentidierPtr id);
    // check typedef names
    TypePtr getTypedefTypes(string name);
    // get detailed type info
    PackedTypeInformationPtr getDetailedTypeInfo(TypePtr ty);
    // get type from model
    TypePtr getTypeFromModel(string name);
    // add new type into model and map
    bool addNewTypeIntoModel(TypePtr ty);
    
}

#endif