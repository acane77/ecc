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
        TypePtr               type;
        StorageClass          storageClass;
        FunctionSpecifierFlag functionSpec;
        TypeQualifierFlag     typeQual;

        PackedTypeInformation(TypePtr t,
            StorageClass sc,
            FunctionSpecifierFlag fs,
            TypeQualifierFlag tq);

        PackedTypeInformationPtr copy();
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

    typedef unordered_map<string, IndexedTypeInformationPtr> TypeMap;
    typedef unordered_map<string, PackedTypeInformationPtr> UnindexedTypeMap;
    typedef shared_ptr<TypeMap> TypeMapPtr;
    typedef shared_ptr<UnindexedTypeMap> UnindexedTypeMapPtr;

    class StructTy {
    public:
        StructType* type;
        TypeMapPtr memberMap;

        using IndexType = IndexedTypeInformation::IndexType;

        template <class... Args>
        StructTy(const TypeMapPtr& tm, string StructName, Args... paramPassToGet) {
            type = StructType::create(paramPassToGet...);
            type->setName(StructName);
            memberMap = tm;
        }
        IndexType getIndex(string memberName);
        
        static map<string, shared_ptr<StructTy>> structs;
        static void saveStruct(shared_ptr<StructTy> ty);
        static shared_ptr<StructTy> get(const string& name);
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

        Identifier(
            string n,
            TypePtr t,
            StorageClass sc,
            FunctionSpecifierFlag fs,
            TypeQualifierFlag tq
         );
    };
}

#endif