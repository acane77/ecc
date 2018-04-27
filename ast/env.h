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

    typedef Value  Identifier;
    typedef Value* IdentifierPtr;

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

        Parse::IParserPtr parser;
    public:
        // instance
        static GlobalScope instance;
        static GlobalScope& getInstance() { return instance; }

    public:
        GlobalScope(): Builder(context) {}
    };

    BasicBlock* switchBasicBlock(string name);
    void switchBasicBlock(BasicBlock* BB);
    LLVMContext& getGlobalContext();
    BasicBlock* getCurrentBasicBlock();
    Function* getCurrentFunction();
    static IRBuilder<>& Builder;
}

#endif