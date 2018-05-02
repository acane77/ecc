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
#include "llvm/IR/Module.h"
#include "llvm/Support/TypeName.h"

namespace Miyuki::AST {
    
    using namespace std;
    using namespace llvm;

    //typedef Value  Identifier;
    //typedef Value* IdentifierPtr;

#define ENABLE_TYPE_DETAIL true

    typedef map<string, TypePtr>   TypedefMap;
    typedef map<string, PackedTypeInformationPtr>   TypeMap;
    typedef map<string, IdentifierPtr> IdentifierMap;
    typedef map<TypePtr, PackedTypeInformationPtr>  DetailedTypeInfo;
    typedef shared_ptr<TypeMap>    TypeMapPtr;

    DEFINE_SHARED_PTR(Scope);

    class Scope : public enable_shared_from_this<Scope> {
        uint32_t      scopeID;
    public:
        IdentifierMap vars;
        ScopePtr      child;
        ScopePtr      parent;

        // for struct/union defines in this scope
        TypeMap       scopedTypes;
        DetailedTypeInfo typeDetail;

        // alias names by typedef
        //   I put alias type names in 2 places - both type info and typedef names
        TypedefMap    typedefs;
    
    public:
        Scope();
        IdentifierPtr getIndentifier(string name);
        bool addIdentifier(const IdentifierPtr& id);
        void enterScope(ScopePtr child);
        void leaveScope();
        IdentifierPtr getIndentifierFromThisScope(string name);
        uint32_t getScopeID();
        TypeMap::value_type::second_type getType(string name);
        bool addType(string name, TypeMap::value_type::second_type ty);
        TypeMap::value_type::second_type getTypeFromThisScope(string name);
        TypedefMap::value_type::second_type getTypedefTy(string name);
        void setTypedefTy(string name, TypedefMap::value_type::second_type ty);
        DetailedTypeInfo::value_type::second_type getDetail(DetailedTypeInfo::key_type ty);
        static Scope* getCuurentScope();
    private:
        //template <class TKey, class TVal>
        //TVal _getElementByName(TKey key);
        IdentifierPtr _getIdentifier(string id);
        TypeMap::value_type::second_type _getType(string name);
        
        static uint32_t __scopeID;
        static Scope* __currentScope;
    };

    class GlobalScope : public Scope {
    public:
        /////////// Parsing-time Information ///////
        // these information generates and be used during compile-time

        // type list of all types
        TypeMapPtr    types;
         
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

    /// Global Functions
    // get global scope ref
    GlobalScope& getGlobalScope();
    // get current scope
    Scope* getCurrentScope();

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
    extern const Module * TheModule;
    
    // ** Compile-Time Information **
    // get identifier by name
    IdentifierPtr getIdentifier(string name);
    // insert identifier to symbol table
    bool addIndentifier(IdentifierPtr id);
    // check typedef names
    TypedefMap::value_type::second_type getTypedefType(string name);
    // set as typedef ty
    TypedefMap::value_type::second_type setTypedefTy(string name, TypedefMap::value_type::second_type ty = nullptr);
    // get detailed type info
    DetailedTypeInfo::value_type::second_type getDetailedTypeInfo(DetailedTypeInfo::key_type ty);
    // get type from scope
    TypeMap::value_type::second_type getTypeFromScope(string name);
    // add new type into model and map
    bool addNewType(TypeMap::value_type::second_type ty);
    // get detail
    DetailedTypeInfo::value_type::second_type getTypeDetail(DetailedTypeInfo::key_type ty);

    /// Template implementation
    /*template<class TKey, class TVal>
    inline TVal Miyuki::AST::Scope::_getElementByName(TKey key) {
        ScopePtr theScope = shared_from_this();
        TVal theValue = nullptr;
        while (theScope != nullptr && theValue == nullptr) {
            theValue = theScope->_getIdentifier(name);
            theScope = theScope->parent;
        }
        return theValue;
    }*/
    
}

#endif