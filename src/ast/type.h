#ifndef _MIYUKI_AST_TYPE_H
#define _MIYUKI_AST_TYPE_H

#include "common/ptrdef.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "lex/token.h"

namespace Miyuki::AST {

    using namespace llvm;
    using namespace Miyuki::Lex;

    typedef Type* TypePtr;
    DEFINE_SHARED_PTR(PackedTypeInformation)
    DEFINE_SHARED_PTR(IndexedTypeInformation)
    DEFINE_SHARED_PTR(StructTy)
    DEFINE_SHARED_PTR(UnionTy)
    DEFINE_SHARED_PTR(Identifier)

    class TypeFactory {
    public:
        static TypePtr build(TokenPtr tok);
    };

    union StorageClass {
        struct {
            bool hasTypedef : 1;
            bool isExtern : 1;
            bool isStatic : 1;
            bool isThreadLocal : 1;
            bool isAuto : 1;
            bool isRegister : 1;
        };
        uint16_t flag = 0;
    };

    union TypeQualifierFlag {
        struct {
            bool isConst : 1;
            bool isRestrict : 1;
            bool isVolatile : 1;
            bool isAtomic : 1;
            bool : 4;
        };
        uint8_t flag = 0;
    };

    union FunctionSpecifierFlag {
        struct {
            bool isInline : 1;
            bool isNoReturn : 1;
        };
        uint8_t flag = 0;
    };

    class PackedTypeInformation {
    public:
        string                typeName;
        TypePtr               type;
        StorageClass          storageClass;
        FunctionSpecifierFlag functionSpec;
        TypeQualifierFlag     typeQual;

        virtual bool isStructTy() { return false; }

        PackedTypeInformation(TypePtr t,
            StorageClass sc,
            FunctionSpecifierFlag fs,
            TypeQualifierFlag tq);
        PackedTypeInformation() {}
        PackedTypeInformation(string name, TypePtr ty) {
            typeName = name;
            type = ty;
        }
        PackedTypeInformationPtr copy();
        void setName(string n) { typeName = n; }
        virtual string getName() { return typeName; }
    };

    class IndexedTypeInformation : public PackedTypeInformation {
    public:
        typedef uint32_t IndexType;
        IndexType index;

        IndexedTypeInformation(TypePtr t,
            StorageClass sc,
            FunctionSpecifierFlag fs,
            TypeQualifierFlag tq,
            IndexType index);
        IndexedTypeInformation(const PackedTypeInformation& base, IndexType index);
    };

    typedef unordered_map<string, IndexedTypeInformationPtr> IndexedTypeMap;
    typedef unordered_map<string, PackedTypeInformationPtr> UnindexedTypeMap;
    typedef shared_ptr<IndexedTypeMap> IndexedTypeMapPtr;
    typedef shared_ptr<UnindexedTypeMap> UnindexedTypeMapPtr;

    class StructTy : public PackedTypeInformation {
    public:
        StructType* type;
        IndexedTypeMapPtr memberMap;

        using IndexType = IndexedTypeInformation::IndexType;

        template <class... Args>
        StructTy(const IndexedTypeMapPtr& tm, string StructName, Args... paramPassToGet) {
            type = StructType::create(paramPassToGet...);
            type->setName(StructName);
            setName(StructName);
            memberMap = tm;
        }
        IndexType getIndex(string memberName);
        
        static map<string, shared_ptr<StructTy>> structs;
        static void saveStruct(shared_ptr<StructTy> ty);
        static shared_ptr<StructTy> get(const string& name);
        bool isStructTy() { return true; }

        // NOTE: struct name is not the name in StructType
        //    we search struct name in type list,
        //    and name in StructType is used in IR represent
    };

    class UnionTy {
    public:
        StructType * type;
        UnindexedTypeMapPtr memberMap;
        string unionName;
        size_t typeSize;

        UnionTy(const UnindexedTypeMapPtr& MM, string name);
        PackedTypeInformationPtr getMember(string name);

        static map<string, shared_ptr<UnionTy>> unions;
        static void saveUnion(shared_ptr<UnionTy> ty);
        static shared_ptr<UnionTy> get(const string& name);
    };

    class TypeUtil {
    public:
        static TypePtr raiseType(TypePtr a, TypePtr b);
        static Value*  createConstant(TokenPtr constTok);
    };

    class Identifier : public PackedTypeInformation {
    public:
        string name;
		Value* addr = nullptr;

        Identifier(
            string n,
            TypePtr t,
            StorageClass sc,
            FunctionSpecifierFlag fs,
            TypeQualifierFlag tq,
			Value* v
         );

        Identifier(string n, TypePtr ty, bool isConst = false, Value* v = nullptr);

		Value* getAddr() { return addr; }
		bool isConstant() { return addr == nullptr; }
    };
}

#endif