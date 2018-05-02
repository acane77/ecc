#include "ast/typename.h"
#include "ast/env.h"
#include "llvm/Support/TypeName.h"

namespace Miyuki::AST {
    using namespace llvm;

    void Miyuki::AST::WithTypeName::setTypeName(string tn) {
        typeName = tn;
    }

    const string & Miyuki::AST::WithTypeName::getTypeName() {
        return typeName;
    }

    string Miyuki::AST::WithTypeName::getAndSetTypeNameFrom(WithTypeNamePtr WTN) {
        setTypeName(WTN->getTypeName());
    }

    string Miyuki::AST::WithTypeName::getPointerTypeName() {
        return typeName + "*";
    }

    string Miyuki::AST::WithTypeName::getConstPointerTypeName() {
        return string("const") + typeName + "*";
    }

    string Miyuki::AST::WithTypeName::getConstTypeName() {
        return string("const") + typeName;
    }

    string Miyuki::AST::WithTypeName::getStructTypeName(string structName) {
        return _getCompName(structName, "struct");
    }

    string Miyuki::AST::WithTypeName::getUnionTypeName(string unionName) {
        return _getCompName(unionName, "union");

    }

    string Miyuki::AST::WithTypeName::_getCompName(string name, string inWhich) {
        Function* F = getCurrentFunction();
        Scope* S = getCurrentScope();
        string FN = F ? F->getName().str() + "::" : string("");
        string SN = F && S ? "${0}"_format(S->getScopeID()) : "";
        return inWhich + " " + name + SN;
    }

    string Miyuki::AST::WithTypeName::getEnumTypeName() {
        return getLLVMTypeName<IntegerType>();
    }

    template<class DesiredType>
    inline string Miyuki::AST::WithTypeName::getLLVMTypeName() {
        return llvm::getTypeName<DesiredType>();
    }
}
