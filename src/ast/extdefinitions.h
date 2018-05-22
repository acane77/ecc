#ifndef _MIYUKI_EXTDEFINITIONS_H
#define _MIYUKI_EXTDEFINITIONS_H

#include "ast/symbols.h"
#include "ast/declaration.h"
#include "ast/statements.h"

// Symbols for external definitions

namespace Miyuki::AST {

    DEFINE_LIST(Declaration);
    DEFINE_SHARED_PTR(TranslationUnit)
    DEFINE_SHARED_PTR(ExternalDeclaration)
    DEFINE_SHARED_PTR(FunctionDefinition)

    class TranslationUnit : public Symbol {
    public:
        ExternalDeclarationPtr extDecl;
        TranslationUnitPtr     unit;

        TranslationUnit(const ExternalDeclarationPtr &extDecl, const TranslationUnitPtr &unit);

		virtual void gen();
        virtual int getKind() override { return Kind::translationUnit; }
    };

    class ExternalDeclaration : public Symbol {
    public:
        FunctionDefinitionPtr funcDef;
        // OR
        DeclarationPtr        decl;

        ExternalDeclaration(const FunctionDefinitionPtr &funcDef);
        ExternalDeclaration(const DeclarationPtr &decl);

		virtual void gen();
        virtual int getKind() override { return Kind::externalDeclaration; }
    };

    class FunctionDefinition : public Symbol {
    public:
        DeclarationSpecifierPtr spec;
        DeclaratorPtr           decr;
        DeclarationListPtr      lst = nullptr;
        CompoundStatementPtr    stmt;

        FunctionDefinition(const DeclarationSpecifierPtr &spec, const DeclaratorPtr &decr,
                           const DeclarationListPtr &lst, const CompoundStatementPtr &stmt);

		virtual void gen();
        virtual int getKind() override { return Kind::functionDefinition; }
    };

}

#endif