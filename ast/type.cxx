#include "type.h"

namespace Miyuki::AST {

    map<uint64_t, TypePtr> TypeFactory::types;

    void TypeFactory::initBasicTypes() {
        // Type(const string& _raw_name, size_t _type_size)
        addType(make_shared<Type>("v", "void", 1));
        addType(make_shared<Type>("i", "int", 4));
        addType(make_shared<Type>("j", "unsigned int", 4));
        addType(make_shared<Type>("l", "long", 4));
        addType(make_shared<Type>("m", "unsigned long", 4));
        addType(make_shared<Type>("x", "long long", 8));
        addType(make_shared<Type>("y", "unsigned long long", 1));
        addType(make_shared<Type>("f", "float", 4));
        addType(make_shared<Type>("d", "double", 8));
        addType(make_shared<Type>("e", "long double", 16));
        addType(make_shared<Type>("c", "char", 1));
        addType(make_shared<Type>("PKc", "const char *", 1));
    }

    TypePtr TypeFactory::build(TokenPtr tok) {
        // NOTE: only use in primary-expression (only constant)
        if ( tok->is(Tag::Integer) ) {
            IntTokenPtr t = static_pointer_cast<IntToken>(tok);
            if ( t->isSigned && t->bit == 16 ) return getType( BasicTypeHashCode::Short );
            if ( t->bit == 16 ) return getType( BasicTypeHashCode::UnsignedShort );
            if ( t->isSigned && t->bit == 32 ) return getType( BasicTypeHashCode::Long );
            if ( t->bit == 32 ) return getType( BasicTypeHashCode::UnsignedLong );
            if ( t->isSigned && t->bit == 64 ) return getType( BasicTypeHashCode::LongLong );
            if ( t->bit == 64 ) return getType( BasicTypeHashCode::UnsignedLong );
            assert(false && "invalid integer token");
        }
        else if ( tok->is(Tag::Character) ) {
            return getType( BasicTypeHashCode::Char );
        }
        else if ( tok->is(Tag::Floating) ) {
            FloatTokenPtr t = static_pointer_cast<FloatToken>(tok);
            if ( t->bit == 32 ) return getType( BasicTypeHashCode::Float );
            if ( t->bit == 64 ) return getType( BasicTypeHashCode::Double );
            assert(false && "invalid floating token");
        }
        return nullptr;
    }

	TypePtr Miyuki::AST::Type::getPointerType() {
		return make_shared<PointerType>(
			string("P") + raw_name,
            string("const ") + name,
			type_size,
			this
			);
	}

    TypePtr Miyuki::AST::Type::getConstType() {
        return make_shared<PointerType>(
            string("K") + raw_name,
            name + "",
            type_size,
            this
            );
    }

	TypePtr Miyuki::AST::Type::getConstPointerType() {
		return make_shared<PointerType>(
			string("PK") + raw_name,
			string("const ") + name + "*",
			type_size,
			this
			);
	}

	Miyuki::AST::FunctionType::FunctionType(TypePtr retType, TypeListPtr _params, bool _isVaArg) {
		params = _params;
		returnType = retType;
		isVaArg = _isVaArg;
		// function raw_name is like: F(return-type)((return-type...))E
		raw_name = "F";
		raw_name += retType->raw_name;
		name = retType->name + " (*) (";
		for (const PackedTypeInformationPtr& para : *params) {
			raw_name += para->type->raw_name;
			name += para->type->name + ",";
		}
		if (!params->size()) {
			raw_name += "v";
			name += "void,";
		}
		raw_name += "E";
		name[name.size() - 1] = ')';
		hash_code = getHashCodeFromRawName(raw_name);
		type_size = TARGET_ARCH_PTR_SIZE;
	}

	Miyuki::AST::StructDef::StructMember::StructMember(string _name, PackedTypeInformationPtr _type) {
		name = _name;
		assert(_type != nullptr && "type == nullptr");
		type = _type->type;
        typeSize = type->type_size;
        typeInfo = _type;
		setTypeSize(type->type_size);
	}

	void Miyuki::AST::StructDef::calculateBias() {
		uint16_t currentBias = 0;
		uint8_t  byteInsideAlignemnt = 0;
		size_t   previousMemberSize = 0;
		for (pair<string, StructMemberPtr> member : members) {
			// calculate align to alignAttribute
			// because member is sorted, so all members with same size with located continuous.
			// just notify size changed.
			size_t typeSize = member.second->type->typeSize;
			if (previousMemberSize != typeSize) {
				previousMemberSize = typeSize;
				if (alignAttribute - byteInsideAlignemnt < typeSize) {
					currentBias = (currentBias % alignAttribute + 1) * alignAttribute;
					byteInsideAlignemnt = typeSize % alignAttribute;
					currentBias += typeSize;
				}
			}
			else {
				member.second->bias = currentBias;
				byteInsideAlignemnt += typeSize;
				byteInsideAlignemnt %= typeSize;
			}
		}
	}

	void Miyuki::AST::StructDef::setAlignAttribute(uint8_t alignAttr) {
		alignAttribute = alignAttr;
		calculateBias();
	}

	StructDef::StructMemberPtr Miyuki::AST::StructDef::getMember(string name) {
		auto it = members.find(name);
		if (it == members.end())
			return nullptr;
		return it->second;
	}

	bool Miyuki::AST::StructDef::addMember(string memberName, StructMemberPtr object) {
		auto it = members.find(memberName);
		if (it == members.end())
			return false;
		members[memberName] = object;
		return true;
	}

	Miyuki::AST::UnionDef::UnionMember::UnionMember(string _name, TypePtr _type) {
		name = _name;
		assert(_type != nullptr && "type == nullptr");
		type = _type;
	}

	UnionDef::UnionMemberPtr Miyuki::AST::UnionDef::getMember(string name) {
		auto it = members.find(name);
		if (it == members.end())
			return nullptr;
		return it->second;
	}

	class InvalidTypeSizeException {};

	void Miyuki::AST::StructDef::StructMember::setTypeSize(size_t size) {
		// check if size is > 0, 2-power, and < original typeSize
		if (size == 0 && (size & size - 1) && size > type->type_size) throw InvalidTypeSizeException();
		typeSize = size;
        bitfieldedSize = size;
	}

    void Miyuki::AST::StructDef::StructMember::setBitField(size_t bitfield) {
        // chck if bifield size greater than its original size
        if (bitfield > typeSize * 8)
            throw InvalidTypeSizeException();
    }

	Enumerators::EnumeratorPtr Miyuki::AST::Enumerators::getEnumerator(string name) {
		auto it = enumerators.find(name);
		if (it == enumerators.end())
			return nullptr;
		return it->second;
	}

    Miyuki::AST::PackedTypeInformation::PackedTypeInformation(TypePtr t, StorageClass sc, FunctionSpecifierFlag fs, TypeQualifierFlag tq)
        : type(t), storageClass(sc), functionSpec(fs), typeQual(tq) {
        
    }

    PackedTypeInformationPtr Miyuki::AST::PackedTypeInformation::copy() {
        return make_shared<PackedTypeInformation>(type, storageClass, functionSpec, typeQual);
    }
}