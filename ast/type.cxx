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

}