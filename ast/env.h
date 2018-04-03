#ifndef _MIYUKI_ENV_H
#define _MIYUKI_ENV_H

#include <map>
#include <vector>
#include "ast/declaration.h"
#include "ast/type.h"
#include "llvm/IR/LLVMContext.h"

namespace Miyuki::AST {
    
    using namespace std;
    using namespace llvm;

    class IdentifierPtr {};

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

        LLVMContext   context;
    public:
        // instance
        static GlobalScope instance;
        static GlobalScope& getInstance() { return instance; }

    public:
        
    };

}

#endif