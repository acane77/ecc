#ifndef _MIYUKI_DECLARATION_H
#define _MIYUKI_DECLARATION_H

#include "symbols.h"
#include "expression.h"

// Declaration contains compile-time information

// ref ISO/IEC 9899:201x °ÏA.2.2

namespace Miyuki::AST {

    using namespace std;
    using namespace Lex;

// define a list of some type
#define DEFINE_LIST(type) typedef deque<type> type##List; typedef shared_ptr<type##List> type##ListPtr;

    DEFINE_SHARED_PTR(IDeclaration)
        DEFINE_SHARED_PTR(Declaration)
        DEFINE_SHARED_PTR(DeclarationSpecifiers)
        DEFINE_SHARED_PTR(InitDeclarator)
        DEFINE_LIST(InitDeclarator)
        DEFINE_SHARED_PTR(DeclarationSpecifier)
        DEFINE_SHARED_PTR(SpecifierAndQualifier)
        DEFINE_LIST(SpecifierAndQualifier)
        DEFINE_SHARED_PTR(StorageClassSpecifier)
        DEFINE_SHARED_PTR(TypeSpecifier)
        DEFINE_SHARED_PTR(TypeQualifier)
        DEFINE_LIST(TypeQualifier)
        DEFINE_SHARED_PTR(StructOrUnionSpecifier)
        DEFINE_SHARED_PTR(StructDeclaration)
        DEFINE_LIST(StructDeclaration)
        DEFINE_SHARED_PTR(StructDeclarator)
        DEFINE_LIST(StructDeclarator)
        DEFINE_SHARED_PTR(Declarator)
        DEFINE_SHARED_PTR(EnumSpecifier)
        DEFINE_SHARED_PTR(Enumerator)
        DEFINE_LIST(Enumerator)
        DEFINE_SHARED_PTR(PointerDecl)
        DEFINE_SHARED_PTR(DirectDeclarator)
        DEFINE_SHARED_PTR(Parameter)
        DEFINE_LIST(WordToken)
        DEFINE_SHARED_PTR(ParameterTypeList)
        DEFINE_SHARED_PTR(ParameterDecleartion)
        DEFINE_LIST(ParameterDecleartion)
        typedef ParameterDecleartionList ParameterList;
        typedef ParameterDecleartionListPtr ParameterListPtr;
        DEFINE_SHARED_PTR(AbstractDeclarator)
        DEFINE_SHARED_PTR(DirectAbstractDeclarator)
        DEFINE_SHARED_PTR(TypeName)
        DEFINE_SHARED_PTR(Initializer)
        DEFINE_SHARED_PTR(InitializerList)
        DEFINE_SHARED_PTR(Designation)
        DEFINE_SHARED_PTR(Designator)
        DEFINE_LIST(Designator)

    class IDeclaration : public Symbol {
    public:
        virtual void gen() { assert(false && "IDeclaration::gen():  unimplemented"); }
    };

    class Declaration : public IDeclaration {
    public:
        DeclarationSpecifiersPtr decSpec;
        InitDeclaratorListPtr    initDeclList;

        // we do not support static_assert
        int getKind() override { return Kind::declaration; }
        void gen() override;
    };

    class DeclarationSpecifier : public IDeclaration {
        SpecifierAndQualifierPtr spec;
        DeclarationSpecifierPtr  decSpec;

        int getKind() override { return Kind::declarationSpecifier; }
        void gen() override;
    };

    class SpecifierAndQualifier : public IDeclaration {
        virtual int getKind() { assert( false && "unimplemented" ); }
        void gen() override;
    };

    class StorageClassSpecifier : public SpecifierAndQualifier {
        TokenPtr tok;

        virtual int getKind() { return Kind::storageClassSpecifier; }
        void gen() override;
    };

    class TypeSpecifier : public SpecifierAndQualifier {
        TokenPtr tok; // INCLUDE typedef-name
        // OR
        StructOrUnionSpecifierPtr structSpec;
        // OR
        EnumSpecifierPtr enumSpec;

        virtual int getKind() { return Kind::typeSpecifier; }
        void gen() override;
    };

    class TypeQualifier : public SpecifierAndQualifier {
        TokenPtr tok;

        virtual int getKind() { return Kind::typeQualifier; }
        void gen() override;
    };

    class FunctionSpecifier : public SpecifierAndQualifier {
        TokenPtr tok;

        virtual int getKind() { return Kind::functionSpecifier; }
        void gen() override;
    };

    class StructOrUnionSpecifier : public TypeSpecifier {
        TokenPtr structOrUnion;
        TokenPtr id;
        StructDeclarationListPtr declList;

        virtual int getKind() { return Kind::structOrUninSpecifier; }
        void gen() override;
    };

    class StructDeclaration: public IDeclaration {
    public:
        SpecifierAndQualifierPtr spec;
        StructDeclarationPtr     structDecl;
        StructDeclaratorListPtr  structDecrList;

        virtual int getKind() { return Kind::structDeclaration; }
        void gen() override;
    };

    class StructDeclarator : public IDeclaration {
    public:
        DeclaratorPtr  decr;
        ConstantExpressionPtr constExpr;

        virtual int getKind() { return Kind::structDeclarator; }
        void gen() override;
    };

    class EnumSpecifier : public IDeclaration {
    public:
        TokenPtr id;
        EnumeratorListPtr enumList;

        virtual int getKind() { return Kind::enumSpecifier; }
        void gen() override;
    };

    class Enumerator : public IDeclaration {
    public:
        TokenPtr enumConstant;
        ConstantExpressionPtr expr;

        virtual int getKind() { return Kind::enumerator; }
        void gen() override;
    };

    class Declarator : public IDeclaration {
    public:
        PointerDeclPtr pointer;
        DirectDeclaratorPtr directDecl;

        virtual int getKind() { return Kind::declarator; }
        void gen() override;
    };

    class PointerDecl : public IDeclaration {
    public:
        TypeQualifierListPtr typeQualList;
        PointerDeclPtr pointerDecl;

        virtual int getKind() { return Kind::pointer_decl; }
        void gen() override;
    };

    class DirectDeclarator : public IDeclaration {
    public:
        TokenPtr id;
        // OR
        DeclaratorPtr decl;
        // OR
        DirectDeclaratorPtr directDecl;
        bool isStatic = false;
        AssignmentExpressionPtr assignExpr;
        bool hasPointer = false;
        ParameterTypeListPtr paramList;
        WordTokenPtr idList;
        TypeQualifierListPtr typeQualList;

        virtual int getKind() { return Kind::directDeclarator; }
        void gen() override;
    };

    class ParameterTypeList : public IDeclaration {
    public:
        ParameterListPtr paramList;
        bool isParameterVarible;

        virtual int getKind() { return Kind::parameterTypeList; }
        void gen() override;
    };

    class ParameterDecleartion : public IDeclaration {
    public:
        DeclarationSpecifierPtr declSpec;
        
        DeclaratorPtr decr;
        //OR
        AbstractDeclaratorPtr abstructDecr;

        virtual int getKind() { return Kind::parameterDecleartion; }
        void gen() override;
    };

    class AbstractDeclarator : public IDeclaration {
    public:
        PointerDeclPtr pointerDecl;
        DirectAbstractDeclaratorPtr directAbstractDecr;

        virtual int getKind() { return Kind::abstractDeclarator; }
        void gen() override;
    };

    class TypeName : public IDeclaration {
    public:
        SpecifierAndQualifierListPtr specList;
        AbstractDeclaratorPtr abstructDecr;

        virtual int getKind() { return Kind::typeName; }
        void gen() override;
    };

    class DirectAbstractDeclarator : public IDeclaration {
    public:
        AbstractDeclaratorPtr abstracrDecr;
        // OR
        DirectAbstractDeclaratorPtr directAbstractDecr;
        bool isStatic = false;
        AssignmentExpressionPtr assignExpr;
        bool hasPointer = false;
        ParameterTypeListPtr paramList;
        TypeQualifierListPtr typeQualList;

        virtual int getKind() { return Kind::directAbstractDeclarator; }
        void gen() override;
    };

    class Initializer : public IDeclaration {
    public:
        AssignmentExpressionPtr assignExpr;
        // OR
        InitializerListPtr      initList;

        virtual int getKind() { return Kind::init; }
        void gen() override;
    };

    class InitializerList : public IDeclaration {
    public:
        InitializerListPtr  initList;
        Initializer         init;
        DesignationPtr      design;

        virtual int getKind() { return Kind::initList; }
        void gen() override;
    };

    class Designation : public IDeclaration {
    public:
        DesignatorListPtr desList;

        virtual int getKind() { return Kind::designation; }
        void gen() override;
    };

    class Designator : public IDeclaration {
    public:
        ConstantExpressionPtr constExpr;
        // OR
        WordTokenPtr id;

        virtual int getKind() { return Kind::designator; }
        void gen() override;
    };

    class InitDeclarator : public IDeclaration {
    public:
        DesignatorPtr desOr;
        Initializer   init;

        virtual int getKind() { return Kind::initDeclr; }
        void gen() override;
    };

}

#endif