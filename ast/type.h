#ifndef _MIYUKI_TYPE_H
#define _MIYUKI_TYPE_H

#include "include.h"
#include "common/ptrdef.h"
#include "common/md5.h"
#include "lex/token.h"
#include <memory>

namespace Miyuki::AST {

    using namespace std;
    using namespace Miyuki::Common;
    using namespace Miyuki::Lex;

    DEFINE_SHARED_PTR(Type)
    DEFINE_SHARED_PTR(StructDef)
    DEFINE_SHARED_PTR(UnionDef)
    DEFINE_SHARED_PTR(ICheckType)

    class Type {
    public:
        uint64_t hash_code;
        string   name;
        string   raw_name;
        size_t   type_size;
        size_t   alignment;
        uint64_t * cvtList;  // note: last element must be 0 //must initial manually

        union TypeInfo {
            struct {
                // type common
                bool isSigned : 1;
                bool isConstPtr : 1;
                bool isPointer : 1;
                bool isArray : 1;
                bool isFunction : 1;
                bool isUnion : 1;
                bool isStruct : 1;
                bool : 1;

                // for every object
                bool isStatic : 1;
                bool isRegister : 1;
                bool isAuto : 1;
                bool isConst : 1;
                bool : 4;
            } info;
            uint16_t flag = 0x8000;
        } typeinfo;

        static uint64_t getHashCodeFromRawName(const string& raw) {
            return md5( raw.c_str() );
        }

        virtual size_t getAlignment() {
            // data structure override this function
            return type_size;
        }

        Type(const string& _raw_name, const string& _name, size_t _type_size) {
            name = _name;
            raw_name = _raw_name;
            type_size = _type_size;
            alignment = getAlignment();
            hash_code = getHashCodeFromRawName(_raw_name);
        }

        bool operator == (const Type& t) { return t.hash_code == hash_code; }
        bool operator != (const Type& t) { return t.hash_code != hash_code; }

        bool isConvertable(uint64_t type) {
            if (type == hash_code) return true;
            for (int i = 0; cvtList[i]; i++) {
                if (cvtList[i] == type)
                    return true;
            }
            return false;
        }

        uint64_t getHashCode() const {
            return hash_code;
        }

    };

    class TypeFactory {
    public:
        static map<uint64_t, TypePtr> types;

        enum BasicTypeHashCode : uint64_t {
            Void = 0x9b675bd57058fd46LLU,
            Int = 0x4ab0e063e5caa338LLU,
            UnsignedInt = 0x528f54df4a0446b6LLU,
            Long = 0x1a9267b7a1188556,
            UnsignedLong = 0x5090da2632453988LLU,
            LongLong = 0x268c8034f5c8564eLLU,
            UnsignedLongLong = 0x9594460e2e485922LLU,
            Short = 0xe395d80182db07aeLLU,
            UnsignedShort = 0x89f58062f10dd731LLU,
            Float = 0x754f91cc6554c9e7LLU,
            Double = 0xd750195b4487976LLU,
            LongDouble = 0xc52e15f763380b45LLU,
            Char = 0x9d37b73795649038LLU,
            ConstCharPtr = 0x7e4efdc972cc1e4dLLU
        };

        // build type for symbols (use in factor)
        //static TypePtr build(TokenPtr tok);
        static TypePtr build(TokenPtr tok);

        static void initBasicTypes();
        static void addType(TypePtr t) { types.insert(make_pair<uint64_t, TypePtr>( (uint64_t)t->hash_code, move(t) )); }
        static TypePtr getType(uint64_t hash_code) {
            auto it = types.find(hash_code);
            if (it == types.end()) return nullptr;
            return it->second;
        }
        static TypePtr getType(const string& rawName) {
            return getType(md5(rawName.c_str()));
        }

    };

    class StructType : public Type {
    public:
        StructDefPtr    structDef;

        StructType(const string& _raw_name, const string& _name, size_t _type_size, StructDefPtr _structDef) : Type(_raw_name, _name, _type_size) {
            structDef = move(_structDef);
            typeinfo.info.isStruct = true;
        }

        size_t getAlignment() final {
            return (type_size >> 2 + (type_size % 4 == 0)) << 2;
        }
    };

    class UnionType : public Type {
    public:
        UnionDefPtr    unionDef;

        UnionType(const string& _raw_name, const string& _name, size_t _type_size, UnionDefPtr _unionDef) : Type(_raw_name, _name, _type_size) {
            unionDef = move(_unionDef);
            typeinfo.info.isUnion = true;
        }

        size_t getAlignment() final {
            return (type_size >> 2 + (type_size % 4 == 0)) << 2;
        }
    };

    class PointerType : public Type {
    public:
        // for pointer and array type (except function pointer)
        // type pointed to or stored in
        TypePtr   baseType = nullptr;

        PointerType(const string& _raw_name, const string& _name, size_t _type_size, TypePtr _base) : Type(_raw_name, _name, _type_size) {
            baseType = move(_base);
            typeinfo.info.isPointer = true;
        }
    };

    class ArrayType : public PointerType {
    public:
        size_t    elementCount;

        ArrayType(const string& _raw_name, const string& _name, size_t _type_size, TypePtr _base, size_t _eleCount) : PointerType(_raw_name, _name, _type_size, move(_base)) {
            elementCount = _eleCount;
            typeinfo.info.isArray = true;
        }
    };

    // interface: class implements this interface can (and must) check type
    class ICheckType {
        TypePtr  _obj_type;
    public:
        bool checkType(const ICheckTypePtr& type) { return checkType(type->_obj_type); }
        bool checkType(const TypePtr& type) { return type->isConvertable( _obj_type->getHashCode() ); }
    };
}

#endif
