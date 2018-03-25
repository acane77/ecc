#ifndef _MIYUKI_ENV_H
#define _MIYUKI_ENV_H

#include <map>
#include <vector>
#include "ast/declaration.h"

namespace Miyuki::AST {
    
    using namespace std;

    class TypePtr {};
    class IdentifierPtr {};

    typedef map<uint64_t, TypePtr> TypeMap;
    typedef map<string, TypePtr>   TypedefMap;
    typedef map<string, IdentifierPtr> IdentifierMap;

    DEFINE_SHARED_PTR(Scope);

    class Scope {
    public:
        IdentifierMap vars;
    };

    class GlobalScope {
    public:
        /////////// Parsing-time Information ///////
        // these information generates and be used during compile-time

        // alias names by typedef
        //   I put alias type names in 2 places - both type info and typedef names
        TypedefMap    typedefs;

        // type list of all types
        TypeMap&      types;

        // intermediate-time dependence scopes
        ScopePtr      scopes = nullptr;

    public:
        // instance
        static GlobalScope instance;
        static GlobalScope& getInstace() { return instance; }

    public:
        
    };

}

#endif