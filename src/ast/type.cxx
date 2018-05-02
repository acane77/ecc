#include "ast/type.h"
#include "ast/env.h"

namespace Miyuki::AST {

    using namespace Miyuki::Lex;
    using namespace llvm;

    TypePtr Miyuki::AST::TypeFactory::build(TokenPtr tok) {
        // NOTE: only use in primary-expression (only constant)
        if (tok->is(Tag::Integer)) {
            IntTokenPtr t = static_pointer_cast<IntToken>(tok);
            if (t->isSigned && t->bit == 16)
                return (Type*)Type::getInt16Ty(GlobalScope::getInstance().context);
            if (t->bit == 16)
                return (Type*)Type::getInt16Ty(GlobalScope::getInstance().context);
            if (t->isSigned && t->bit == 32)
                return (Type*)Type::getInt32Ty(GlobalScope::getInstance().context);
            if (t->bit == 32)
                return (Type*)Type::getInt32Ty(GlobalScope::getInstance().context);
            if (t->isSigned && t->bit == 64)
                return (Type*)Type::getInt64Ty(GlobalScope::getInstance().context);
            if (t->bit == 64)
                return (Type*)Type::getInt64Ty(GlobalScope::getInstance().context);
            assert(false && "invalid integer token");
        }
        else if (tok->is(Tag::Character)) {
            return (Type*)Type::getInt8Ty(GlobalScope::getInstance().context);
        }
        else if (tok->is(Tag::Floating)) {
            FloatTokenPtr t = static_pointer_cast<FloatToken>(tok);
            if (t->bit == 32)
                return Type::getFloatTy(GlobalScope::getInstance().context);
            if (t->bit == 64)
                return Type::getDoubleTy(GlobalScope::getInstance().context);
            assert(false && "invalid floating token");
        }
        else if (tok->is(Tag::StringLiteral)) {
            return (Type*)Type::getInt8PtrTy(GlobalScope::getInstance().context);
        }
        return nullptr;
    }

    Miyuki::AST::PackedTypeInformation::PackedTypeInformation(TypePtr t, StorageClass sc, FunctionSpecifierFlag fs, TypeQualifierFlag tq)
        : type(t), storageClass(sc), functionSpec(fs), typeQual(tq) {

    }

    PackedTypeInformationPtr Miyuki::AST::PackedTypeInformation::copy() {
        return make_shared<PackedTypeInformation>(type, storageClass, functionSpec, typeQual);
    }

    map<string, shared_ptr<StructTy>> StructTy::structs;
    void Miyuki::AST::StructTy::saveStruct(shared_ptr<StructTy> ty) {
        assert(ty && "ty == nullptr");
        structs[ty->type->getName()] = ty;
    }

    shared_ptr<StructTy> Miyuki::AST::StructTy::get(const string& name) {
        auto it = structs.find(name);
        return it == structs.end() ? nullptr : it->second;
    }

    StructTy::IndexType Miyuki::AST::StructTy::getIndex(string memberName) {
        auto it = memberMap->find(memberName);
        return it == memberMap->end() ? -1 : it->second->index;
    }

    Miyuki::AST::IndexedTypeInformation::IndexedTypeInformation(TypePtr t, StorageClass sc, FunctionSpecifierFlag fs, TypeQualifierFlag tq, IndexType index)
        :PackedTypeInformation(t, sc, fs, tq), index(index) {
    }

    IndexedTypeInformation::IndexedTypeInformation(const PackedTypeInformation & base, IndexType index)
        :PackedTypeInformation(base), index(index) {
    }

    UnionTy::UnionTy(const UnindexedTypeMapPtr & MM, string name) {
        memberMap = MM;
        unionName = name;
        typeSize = 0;
        for (auto e : *memberMap) {
            size_t S = e.second->type->getScalarSizeInBits();
            if (S > typeSize) typeSize = S;
        }
    }

    map<string, shared_ptr<UnionTy>> UnionTy::unions;
    
    void UnionTy::saveUnion(shared_ptr<UnionTy> ty) {
        assert(ty && "ty == nullptr");
        unions[ty->unionName] = ty;
    }

    shared_ptr<UnionTy> UnionTy::get(const string & name) {
        auto it = unions.find(name);
        return it == unions.end() ? nullptr : it->second;
    }

    PackedTypeInformationPtr UnionTy::getMember(string name) {
        auto it = memberMap->find(name);
        return it == memberMap->end() ? nullptr : it->second;
    }

    TypePtr TypeUtil::raiseType(TypePtr a, TypePtr b) {
        // if is same type
        if (a == b)
            return a;

        // one of the two is pointer type
        if (a->isPointerTy() && !b->isPointerTy()) a = Type::getInt32Ty(GlobalScope::getInstance().context);
        if (b->isPointerTy() && !a->isPointerTy()) b = Type::getInt32Ty(GlobalScope::getInstance().context);

        // or a and b are both pointer type
        if (a->isPointerTy() && b->isPointerTy()) {
            if (a == b) return a;
            else return nullptr; //error
        }
        cout << "[ScalarSizeInBits] " << a->getScalarSizeInBits() << "  " << b->getScalarSizeInBits() << "\n";
        if (a->getScalarSizeInBits() > b->getScalarSizeInBits()) {
            if (a->isDoubleTy()) return a;
            if (b->isFloatTy()) return b; // with int64_t and float
            return a;
        }
        else if (a->getScalarSizeInBits() < b->getScalarSizeInBits()) {
            if (a->isDoubleTy()) return a; 
            if (b->isDoubleTy()) return b; // with double and float
            if (a->isFloatTy()) return a;
            if (b->isFloatTy()) return b; // with int64_t and float
            return b;
        }
        else {  
            if (a->isFloatTy() || a->isDoubleTy())
                return b->isDoubleTy() ? b : a;
            return b;
        }
        return nullptr;
    }

    Identifier::Identifier(string n, TypePtr t, StorageClass sc, FunctionSpecifierFlag fs, TypeQualifierFlag tq)
        : PackedTypeInformation(t, sc, fs, tq) {
        name = n;
    }
}

