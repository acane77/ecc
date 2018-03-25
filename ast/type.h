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
	DEFINE_SHARED_PTR(StructType)
	DEFINE_SHARED_PTR(UnionType)
	DEFINE_SHARED_PTR(StructDef)  // TODO: implement this.
	DEFINE_SHARED_PTR(UnionDef)   // TODO: implement this.
	DEFINE_SHARED_PTR(FunctionType)
    DEFINE_SHARED_PTR(ICheckType)
    DEFINE_SHARED_PTR(PackedTypeInformation)

#define TYPE_QUALIFIER_RAWNAME_CONST "K"
#define TYPE_QUALIFIER_RAWNAME_OTHERS ""

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
			bool isInline: 1;
			bool isNoReturn: 1;
		};
		uint8_t flag = 0;
	};

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
            } ;
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

        TypePtr getPointerType();
        TypePtr getConstType();
		TypePtr getConstPointerType();

		void setRawName(string&& name) {
			hash_code = getHashCodeFromRawName(name);
			raw_name = move(name);
		}

		friend class TypeFactory;
	protected:
		Type() { }
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

	typedef Type::TypeInfo TypeFlag;
	typedef deque<PackedTypeInformationPtr> TypeList;
	typedef shared_ptr<TypeList> TypeListPtr;

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
        static void addType(TypePtr t) { types[t->hash_code] = t; }
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
            typeinfo.isStruct = true;
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
            typeinfo.isUnion = true;
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
            typeinfo.isPointer = true;
        }

		TypePtr getBaseType() { return baseType; }
    };

    class ArrayType : public PointerType {
    public:
        size_t    elementCount;

        ArrayType(const string& _raw_name, const string& _name, size_t _type_size, TypePtr _base, size_t _eleCount) : PointerType(_raw_name, _name, _type_size, move(_base)) {
            elementCount = _eleCount;
            typeinfo.isArray = true;
        }
    };

	class FunctionType : public Type {
	public:
		TypePtr returnType;
		TypeListPtr params;
		bool isVaArg;

		FunctionType(TypePtr retType, TypeListPtr _params, bool isVaArg);
        
	};

    // interface: class implements this interface can (and must) check type
    class ICheckType {
        TypePtr  _obj_type;
    public:
        bool checkType(const ICheckTypePtr& type) { return checkType(type->_obj_type); }
        bool checkType(const TypePtr& type) { return type->isConvertable( _obj_type->getHashCode() ); }
    };

	// struct definition
	class StructDef {
	public:
		class StructMember {
		public:
			string name;
			TypePtr type;
			uint16_t bias = -1; // bias unset
			size_t typeSize;
            float  bitfieldedSize;
            PackedTypeInformationPtr typeInfo;

			StructMember(string _name, PackedTypeInformationPtr _type);

			bool operator <(StructMember& m) { return m.type->type_size < m.type->type_size; }
			bool operator ==(StructMember& m) { return m.type->type_size == m.type->type_size; }
			bool operator <=(StructMember& m) { return m.type->type_size < m.type->type_size; }
			bool operator >(StructMember& m) { return m.type->type_size > m.type->type_size; }
			bool operator !=(StructMember& m) { return m.type->type_size != m.type->type_size; }
			bool operator >=(StructMember& m) { return m.type->type_size >= m.type->type_size; }

			void setTypeSize(size_t size);
            void setBitField(size_t bitfield);
		};
		typedef shared_ptr<StructMember> StructMemberPtr;

		// Vector will sort autometicly, and using calculateBias() to get bias after sorted(inserted). 
		typedef std::map<string, StructMemberPtr> StructMemberList;
		StructMemberList members;
		uint8_t alignAttribute = 4;
		void calculateBias();
		void setAlignAttribute(uint8_t alignAttr);
		StructMemberPtr getMember(string name);
		bool addMember(string memberName, StructMemberPtr object);
	};

	class UnionDef {
	public:
		class UnionMember {
		public:
			string name;
			TypePtr type;

			UnionMember(string _name, TypePtr _type);
		};
		typedef shared_ptr<UnionMember> UnionMemberPtr;

		typedef std::unordered_map<string, UnionMemberPtr> UnionMemberList;
		UnionMemberList members;
		UnionMemberPtr getMember(string name);
	};

	// when find identifiers, also find in enumeratiors.
	class Enumerators {
	public:
		class EnumMember {
		public:
			string name;
			TypePtr type;
			union {
				int64_t value64;
				uint64_t uvalue64;
				int32_t value;
				uint32_t uvalue;
				int16_t value16;
				uint16_t uvalue16;
				int8_t value8;
				uint8_t uvalue8;
			} value;
		};
		typedef shared_ptr<Enumerator> EnumeratorPtr;
		typedef std::unordered_map<string, EnumeratorPtr> EnumMemberList;
		EnumMemberList enumerators;
		EnumeratorPtr getEnumerator(string name);
	};
}

#endif
