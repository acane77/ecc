#include "env.h"
#include "llvm/IR/Type.h"

namespace Miyuki::AST {
    GlobalScope GlobalScope::instance;
    IRBuilder<>& Builder = GlobalScope::instance.Builder;
    const size_t PointerSize = 4;
    Module __M_CXX_Model("ecc", getGlobalContext());
    const Module* TheModule = &__M_CXX_Model;

    GlobalScope & getGlobalScope() {
        return GlobalScope::instance;
    }

    LLVMContext & getGlobalContext() {
        return GlobalScope::getInstance().context;
    }

    BasicBlock * getCurrentBasicBlock() {
        return GlobalScope::getInstance().currentBasicBlock;
    }

    Function * getCurrentFunction() {
        return GlobalScope::getInstance().currentFunction;
    }

    BasicBlock* switchBasicBlock(string name) {
        BasicBlock* BB = BasicBlock::Create(getGlobalContext(), name, getCurrentFunction());
        Builder.SetInsertPoint(BB);
        GlobalScope::getInstance().currentBasicBlock = BB;
        return BB;
    }

    void switchBasicBlock(BasicBlock * BB) {
        BasicBlock* BBC = getCurrentBasicBlock();
        Function*   F = getCurrentFunction();
        assert(F && "Not in a function");
        BBC->insertInto(F);
        Builder.SetInsertPoint(BB);
        GlobalScope::getInstance().currentBasicBlock = BB;
    }

    // Scope
    uint32_t Scope::__scopeID = 0;
    Scope* Scope::__currentScope = &GlobalScope::instance;
    DetailedTypeInfo Scope::typeDetail;

    Miyuki::AST::Scope::Scope() {
        scopeID = __scopeID++;
    }

    uint32_t Miyuki::AST::Scope::getScopeID() {
        return scopeID;
    }

    IdentifierPtr Miyuki::AST::Scope::getIndentifier(string name) {
        ScopePtr theScope = shared_from_this();
        IdentifierPtr theID = nullptr;
        while (theScope != nullptr && theID == nullptr) {
            theID = theScope->_getIdentifier(name);
            theScope = theScope->parent;
        }
        return theID;
    }

    IdentifierPtr Miyuki::AST::Scope::_getIdentifier(string id) {
        auto it = vars.find(id);
        if (it != vars.end())
            return it->second;
        return nullptr;
    }

    IdentifierPtr Miyuki::AST::Scope::getIndentifierFromThisScope(string name) {
        return Scope::_getIdentifier(name);
    }

    bool Miyuki::AST::Scope::addIdentifier(const IdentifierPtr & id) {
        // id exists
        const string& name = id->name;
        if (_getIdentifier(name) != nullptr) return false;
        vars[name] = id;
        return true;
    }

    void Miyuki::AST::Scope::enterScope(ScopePtr c) {
        c->parent = shared_from_this();
        child = c;
        Scope::__currentScope = c.get();
    }

	ScopePtr Miyuki::AST::Scope::enterScope() {
		ScopePtr c = make_shared<Scope>();
		enterScope(c);
		return c;
	}

    void Miyuki::AST::Scope::leaveScope() {
        child->parent = nullptr;
        child = nullptr;
        Scope::__currentScope = this;
    }

    ScopePtr Miyuki::AST::Scope::getCuurentScope() {
        return Scope::__currentScope;
    }

    Scope * getCurrentScope() {
        return Scope::getCuurentScope();
    }

    TypeMap::value_type::second_type Miyuki::AST::Scope::getTypeFromThisScope(string name) {
        return Scope::_getType(name);
    }

    TypeMap::value_type::second_type Miyuki::AST::Scope::_getType(string name) {
        auto it = scopedTypes.find(name);
        if (it != scopedTypes.end())
            return it->second;
        return nullptr;
    }

    TypeMap::value_type::second_type Miyuki::AST::Scope::getType(string name) {
        ScopePtr theScope = shared_from_this();
        TypeMap::value_type::second_type theType = nullptr;
        while (theScope != nullptr && theType == nullptr) {
            theType = theScope->_getType(name);
            theScope = theScope->parent;
        }
        return theType;
    }

    bool Miyuki::AST::Scope::addType(string name, TypeMap::value_type::second_type ty) {
        // type exists
        if (_getType(name) != nullptr) 
            return false;
        scopedTypes[name] = ty;
        typeDetail[ty->type] = ty;
        return true;
    }

    IdentifierPtr getIdentifier(string name) {
        return getCurrentScope()->getIndentifier(name);
    }

    bool addIdentifier(IdentifierPtr id) {
        return getCurrentScope()->addIdentifier(id);
    }

    TypedefMap::value_type::second_type getTypedefType(string name) {
        return getCurrentScope()->getTypedefTy(name);
    }

    void Miyuki::AST::Scope::setTypedefTy(string name, TypedefMap::value_type::second_type ty) {
        typedefs[name] = ty;
    }

    TypedefMap::value_type::second_type setTypedefTy(string name, TypedefMap::value_type::second_type ty) {
        getCurrentScope()->setTypedefTy(name, ty);
        return ty;
    }

    TypedefMap::value_type::second_type Miyuki::AST::Scope::getTypedefTy(string name) {
        ScopePtr theScope = shared_from_this();
        TypedefMap::value_type::second_type theType = nullptr;
        while (theScope != nullptr && theType == nullptr) {
            auto _t = theScope->_getType(name);
            theType = _t ? _t->type : nullptr;
            theScope = theScope->parent;
        }
        return theType;
    }

    DetailedTypeInfo::value_type::second_type getDetailedTypeInfo(DetailedTypeInfo::key_type ty) {
        return getCurrentScope()->getDetail(ty);
    }

    DetailedTypeInfo::value_type::second_type Miyuki::AST::Scope::getDetail(DetailedTypeInfo::key_type ty) {
        auto it = typeDetail.find(ty);
        if (it == typeDetail.end())
            return nullptr;
        return it->second;
    }

    TypeMap::value_type::second_type getTypeFromScope(string name) {
        return getCurrentScope()->getType(name);
    }

    bool addNewType(TypeMap::value_type::second_type ty) {
        assert(ty->typeName != "" && "a type name must specified");
        return getCurrentScope()->addType(ty->typeName, ty);
    }

    DetailedTypeInfo::value_type::second_type getTypeDetail(DetailedTypeInfo::key_type ty) {
        return getCurrentScope()->getDetail(ty);
    }
}