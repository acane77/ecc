#include "type.h"

namespace Miyuki::AST {

    map<uint64_t, TypePtr> TypeFactory::types;

    void TypeFactory::initBasicTypes() {
        // Type(const string& _raw_name, size_t _type_size)
        addType(make_shared<Type>("C", "char", 1)); // char
        addType(make_shared<Type>("V", "void", 1)); // void
        addType(make_shared<Type>("I", "int", 4)); // int
        addType(make_shared<Type>("L", "long", 4)); // long
        addType(make_shared<Type>("Q", "long long", 8)); // long long
        addType(make_shared<Type>("H", "short", 1)); // short
        addType(make_shared<Type>("S", "float", 4)); // float
        addType(make_shared<Type>("D", "double", 8)); // double
        addType(make_shared<Type>("K", "long double", 16)); // long double
    }
}