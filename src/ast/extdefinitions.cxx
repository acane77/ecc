#include "ast/extdefinitions.h"

namespace Miyuki::AST {

    TranslationUnit::TranslationUnit(const ExternalDeclarationPtr &extDecl, const TranslationUnitPtr &unit) :
            extDecl(extDecl), unit(unit) {}

    ExternalDeclaration::ExternalDeclaration(const FunctionDefinitionPtr &funcDef) : funcDef(funcDef) {}

    ExternalDeclaration::ExternalDeclaration(const DeclarationPtr &decl) : decl(decl) {}

    FunctionDefinition::FunctionDefinition(const DeclarationSpecifierPtr &spec, const DeclaratorPtr &decr,
                                           const DeclarationListPtr &lst, const CompoundStatementPtr &stmt) :
            spec(spec), decr(decr), lst(lst), stmt(stmt) {}
}