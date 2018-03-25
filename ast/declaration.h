#ifndef _MIYUKI_DECLARATION_H
#define _MIYUKI_DECLARATION_H

#include "symbols.h"
#include "expression.h"

// Declaration contains compile-time information

// ref ISO/IEC 9899:201x ��A.2.2

namespace Miyuki::AST {

    using namespace std;
    using namespace Lex;

    DEFINE_SHARED_PTR(IDeclaration)
    DEFINE_SHARED_PTR(Declaration)
    DEFINE_SHARED_PTR(InitDeclarator)
    DEFINE_LIST(InitDeclarator)
    DEFINE_SHARED_PTR(DeclarationSpecifier)
    DEFINE_SHARED_PTR(SpecifierAndQualifier)
    DEFINE_LIST(SpecifierAndQualifier)
    DEFINE_SHARED_PTR(StorageClassSpecifier)
	DEFINE_SHARED_PTR(FunctionSpecifier)
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
	DEFINE_SHARED_PTR(IDeclarator)
	DEFINE_SHARED_PTR(IDirectDeclarator)
	typedef string& StringRef;

    class IDeclaration : public Symbol {
    public:
        virtual void gen() { assert(false && "IDeclaration::gen():  unimplemented"); }
    };

    class IDeclarator : public IDeclaration {
    public:
        virtual void gen() { assert(false && "not implemented"); }
    };

    class IDirectDeclarator : public IDeclaration {
    public:
        virtual void gen() { assert(false && "not implemented"); }
    };

    class Declaration : public IDeclaration {
    public:
        DeclarationSpecifierPtr decSpec = nullptr;
        InitDeclaratorListPtr    initDeclList = nullptr;

        Declaration(const DeclarationSpecifierPtr &decSpec, const InitDeclaratorListPtr &initDeclList);

        // we do not support static_assert
        int getKind() override { return Kind::declaration; }
        virtual void gen() {}
    };

    class DeclarationSpecifier : public IDeclaration {
    public:
        SpecifierAndQualifierPtr spec = nullptr;
        DeclarationSpecifierPtr  decSpec = nullptr;

        DeclarationSpecifier(const SpecifierAndQualifierPtr &spec, const DeclarationSpecifierPtr &decSpec);

        int getKind() override { return Kind::declarationSpecifier; }
        virtual void gen() {}
        SpecifierAndQualifierListPtr generateSpecifierQualifierList();
        void _genSpecQualList(SpecifierAndQualifierListPtr lst);
    };

    class SpecifierAndQualifier : public IDeclaration {
	public:
        virtual int getKind() { assert( false && "unimplemented" ); }
        virtual void gen() { assert ( false && "call getTypeName() instead" ); }
		virtual StorageClass getStorageClass();
		virtual void getTypeQualifier(TypeQualifierFlag& flag) { assert(false && "unimplemented"); }
		virtual TypePtr getType() { assert(false && "unimplemented"); }
		virtual bool isStorageClassSpecifier() { return false; }
		virtual bool isTypeQualifier() { return false; }
		virtual bool isFunctionSpecifier() { return false; }
		virtual bool isTypeSpecifier() { return false; }
	};

    class StorageClassSpecifier : public SpecifierAndQualifier {
    public:
        TokenPtr tok = nullptr;

        explicit StorageClassSpecifier(const TokenPtr &tok);

        virtual int getKind() { return Kind::storageClassSpecifier; }
        virtual void gen() {}
		virtual StorageClass getStorageClass();
		virtual bool isStorageClassSpecifier() { return true; }
    };

    class TypeSpecifier : public SpecifierAndQualifier {
    public:
        TokenPtr tok = nullptr; // INCLUDE typedef-name
        // OR
        // is enum or struxt or union (derived class)

        explicit TypeSpecifier(const TokenPtr &tok);
        explicit TypeSpecifier();

        virtual int getKind() { return Kind::typeSpecifier; }
        virtual void gen() { assert ( false && "unimplemented" ); }
		virtual TypePtr getType();
    };

    class TypeQualifier : public SpecifierAndQualifier {
    public:
        TokenPtr tok = nullptr;

        explicit TypeQualifier(const TokenPtr &tok);

        virtual int getKind() { return Kind::typeQualifier; }
		virtual void getTypeQualifier(TypeQualifierFlag& flag);
		virtual bool isTypeQualifier() { return true; }
    };

    class FunctionSpecifier : public SpecifierAndQualifier {
    public:
        TokenPtr tok = nullptr;

        explicit FunctionSpecifier(const TokenPtr &tok);

        virtual int getKind() { return Kind::functionSpecifier; }
        virtual void gen() {}
		virtual FunctionSpecifierFlag getFunctionSpecifier();
		virtual bool isFunctionSpecifier() { return true; }
    };

    class StructOrUnionSpecifier : public TypeSpecifier {
    public:
        TokenPtr structOrUnion = nullptr;
        TokenPtr id = nullptr;
        StructDeclarationListPtr declList = nullptr;

        StructOrUnionSpecifier(const TokenPtr &structOrUnion, const TokenPtr &id, const StructDeclarationListPtr &declList);

        virtual int getKind() { return Kind::structOrUninSpecifier; }
        virtual void gen() {}
		virtual TypePtr getType(const TypeQualifierFlag& flag);
    };

    class StructDeclaration: public IDeclaration {
    public:
        SpecifierAndQualifierListPtr specList = nullptr;
        StructDeclaratorListPtr  structDecrList = nullptr;

        StructDeclaration(const SpecifierAndQualifierListPtr &specList, const StructDeclaratorListPtr &structDecrList);

        virtual int getKind() { return Kind::structDeclaration; }
        virtual void gen() {}
		virtual void getMemberInfo(StructDefPtr structDef);
    };

    class StructDeclarator : public IDeclaration {
    public:
        DeclaratorPtr  decr = nullptr;
        ConstantExpressionPtr constExpr = nullptr;

        explicit StructDeclarator(const DeclaratorPtr &decr, const ConstantExpressionPtr &constExpr = nullptr);

        virtual int getKind() { return Kind::structDeclarator; }
        virtual void gen() {}

        // eg. int ** x;
        //    (int) is baseType
        //    (int **) is its real type
        TypePtr getType(TypePtr basetype);
        // can also have no name (with IDeclarator = AbstractDeclarator)
        TokenPtr getMemberNameTok();
    };

    class EnumSpecifier : public TypeSpecifier {
    public:
        TokenPtr id = nullptr;
        EnumeratorListPtr enumList = nullptr;

        EnumSpecifier(const TokenPtr &id, const EnumeratorListPtr &enumList);

        virtual int getKind() { return Kind::enumSpecifier; }
        virtual void gen() {}
		virtual TypePtr getType(const TypeQualifierFlag& flag);
    };

    class Enumerator : public IDeclaration {
    public:
        TokenPtr enumConstant = nullptr;
        ConstantExpressionPtr expr = nullptr;

        explicit Enumerator(const TokenPtr &enumConstant, const ConstantExpressionPtr &expr = nullptr);

        virtual int getKind() { return Kind::enumerator; }
        virtual void gen() {}
    };

    class Declarator : public IDeclarator {
    public:
        PointerDeclPtr pointer = nullptr;
        DirectDeclaratorPtr directDecl = nullptr;

        explicit Declarator(const DirectDeclaratorPtr &directDecl);
        Declarator(const PointerDeclPtr &pointer, const DirectDeclaratorPtr &directDecl);

        TypePtr getType(TypePtr baseType);
        TokenPtr getMemberNameTok();

        virtual int getKind() { return Kind::declarator; }
        virtual void gen() {}
    };
     
    class PointerDecl : public IDeclaration {
    public:
        TypeQualifierListPtr typeQualList = nullptr;
        PointerDeclPtr pointerDecl = nullptr;

        PointerDecl(const TypeQualifierListPtr &typeQualList, const PointerDeclPtr &pointerDecl);
        PointerDecl(const PointerDeclPtr &pointerDecl);

        virtual int getKind() { return Kind::pointer_decl; }
        virtual void gen() {}
		TypePtr getType(TypePtr baseType);
    };

    class DirectDeclarator : public IDirectDeclarator {
    public:
        TokenPtr id = nullptr;
        // OR
        DeclaratorPtr decl = nullptr;
        // OR
        DirectDeclaratorPtr directDecl = nullptr;
        bool isStatic = false;
        AssignmentExpressionPtr assignExpr = nullptr;
        bool hasPointer = false;
        ParameterTypeListPtr paramList = nullptr;
        WordTokenListPtr idList = nullptr;
        TypeQualifierListPtr typeQualList = nullptr;

        // production to be used (ref 6.7.6)
        int productionID;
        bool isArrayDeclarator = false;
        bool isFunctionPrototypeDeclaration = false;
        bool isOldStyleFunctionPrototypeDeclaration = false;

        explicit DirectDeclarator(const TokenPtr &id);
        explicit DirectDeclarator(const DeclaratorPtr &decl);
        DirectDeclarator(const DirectDeclaratorPtr &directDecl, bool isStatic,
                         const AssignmentExpressionPtr &assignExpr, const TypeQualifierListPtr &typeQualList,
                         int productionID = 2);
        DirectDeclarator(const DirectDeclaratorPtr &directDecl, const TypeQualifierListPtr &typeQualList);
        DirectDeclarator(const DirectDeclaratorPtr &directDecl, const ParameterTypeListPtr &paramList);
        DirectDeclarator(const DirectDeclaratorPtr &directDecl, const WordTokenListPtr &idList);

        virtual int getKind() { return Kind::directDeclarator; }
        virtual void gen() {}
        virtual TypePtr getType(TypePtr baseType);
        TokenPtr getMemberNameTok();
    };

    class ParameterTypeList : public IDeclaration {
    public:
        ParameterListPtr paramList = nullptr;
        bool isParameterVarible;

        explicit ParameterTypeList(const ParameterListPtr &paramList, bool isParameterVarible = false);

        virtual int getKind() { return Kind::parameterTypeList; }
        virtual void gen() {}
        void generateTypeList(TypeListPtr lst);
    };

    class ParameterDecleartion : public IDeclaration {
    public:
        DeclarationSpecifierPtr declSpec = nullptr;
        
        DeclaratorPtr decr = nullptr;
        //OR
        AbstractDeclaratorPtr abstructDecr = nullptr;

        ParameterDecleartion(const DeclarationSpecifierPtr &declSpec, const DeclaratorPtr &decr);

        explicit ParameterDecleartion(const DeclarationSpecifierPtr &declSpec, const AbstractDeclaratorPtr &abstructDecr = nullptr);

        virtual int getKind() { return Kind::parameterDecleartion; }
        virtual void gen() {}
        virtual PackedTypeInformationPtr getTypeInfo();
    };

    class AbstractDeclarator : public IDeclarator {
    public:
        PointerDeclPtr pointerDecl = nullptr;
        DirectAbstractDeclaratorPtr directAbstractDecr = nullptr;

        AbstractDeclarator(const PointerDeclPtr &pointerDecl, const DirectAbstractDeclaratorPtr &directAbstractDecr);

        virtual int getKind() { return Kind::abstractDeclarator; }
        virtual void gen() {}
        TypePtr getType(TypePtr baseType);
    };

    class TypeName : public IDeclaration {
    public:
        SpecifierAndQualifierListPtr specList = nullptr;
        AbstractDeclaratorPtr abstructDecr = nullptr;

        TypeName(const SpecifierAndQualifierListPtr &specList, const AbstractDeclaratorPtr &abstructDecr);

        virtual int getKind() { return Kind::typeName; }
		virtual void gen() { assert(false && "call getTypeName() instead"); }
		PackedTypeInformationPtr getType();
    };

    class DirectAbstractDeclarator : public IDirectDeclarator {
    public:
        AbstractDeclaratorPtr abstracrDecr = nullptr;
        // OR
        DirectAbstractDeclaratorPtr directAbstractDecr = nullptr;
        bool isStatic = false;
        AssignmentExpressionPtr assignExpr = nullptr;
        bool hasPointer = false;
        ParameterTypeListPtr paramList = nullptr;
        TypeQualifierListPtr typeQualList = nullptr;

        // ref 6.7.7   production ID
        int productionID;
        bool isArrayDeclarator = false;
        bool isFunctionPrototypeDeclaration = false;
        bool isOldStyleFunctionPrototypeDeclaration = false;

        explicit DirectAbstractDeclarator(const AbstractDeclaratorPtr &abstracrDecr);
        DirectAbstractDeclarator(const DirectAbstractDeclaratorPtr &directAbstractDecr,
                                 const AssignmentExpressionPtr &assignExpr, const TypeQualifierListPtr &typeQualList, bool _isStatic, int _productionID = 1);
        explicit DirectAbstractDeclarator(const DirectAbstractDeclaratorPtr &directAbstractDecr);
        DirectAbstractDeclarator(const DirectAbstractDeclaratorPtr &directAbstractDecr, const ParameterTypeListPtr &paramList);

        virtual int getKind() { return Kind::directAbstractDeclarator; }
        virtual void gen() {}
		TypePtr getType(TypePtr baseType);
    };

    class Initializer : public IDeclaration {
    public:
        AssignmentExpressionPtr assignExpr = nullptr;
        // OR
        InitializerListPtr      initList = nullptr;

        explicit Initializer(const AssignmentExpressionPtr &assignExpr);
        explicit Initializer(const InitializerListPtr &initList);

        virtual int getKind() { return Kind::init; }
        virtual void gen() {}
    };

    class InitializerList : public IDeclaration {
    public:
        InitializerListPtr  initList = nullptr;
        InitializerPtr      init = nullptr;
        DesignationPtr      design = nullptr;

        InitializerList(const InitializerPtr &init, const DesignationPtr &design, const InitializerListPtr& initList = nullptr);

        virtual int getKind() { return Kind::initList; }
        virtual void gen() {}
    };

    class Designation : public IDeclaration {
    public:
        DesignatorListPtr desList = nullptr;

        explicit Designation(const DesignatorListPtr &desList);

        virtual int getKind() { return Kind::designation; }
        virtual void gen() {}
    };

    class Designator : public IDeclaration {
    public:
        ConstantExpressionPtr constExpr = nullptr;
        // OR
        WordTokenPtr id = nullptr;

        explicit Designator(const ConstantExpressionPtr &constExpr);
        explicit Designator(const WordTokenPtr &id);

        virtual int getKind() { return Kind::designator; }
        virtual void gen() {}
    };

    class InitDeclarator : public IDeclaration {
    public:
        DeclaratorPtr desOr = nullptr;
        InitializerPtr init = nullptr;

        explicit InitDeclarator(const DeclaratorPtr &desOr, const InitializerPtr &init = nullptr);

        virtual int getKind() { return Kind::initDeclr; }
        virtual void gen() {}
    };

}

#endif