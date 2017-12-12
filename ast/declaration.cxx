#include "declaration.h"

namespace Miyuki::AST {

    Declaration::Declaration(const DeclarationSpecifiersPtr &decSpec, const InitDeclaratorListPtr &initDeclList)
            : decSpec(decSpec), initDeclList(initDeclList) {}

    DeclarationSpecifier::DeclarationSpecifier(const SpecifierAndQualifierPtr &spec,
                                               const DeclarationSpecifierPtr &decSpec) : spec(spec), decSpec(decSpec) {}

    StorageClassSpecifier::StorageClassSpecifier(const TokenPtr &tok) : tok(tok) {}

    TypeSpecifier::TypeSpecifier(const TokenPtr &tok) : tok(tok) {}

    TypeSpecifier::TypeSpecifier(const EnumSpecifierPtr &enumSpec) : enumSpec(enumSpec) {}

    TypeSpecifier::TypeSpecifier(const StructOrUnionSpecifierPtr &structSpec) : structSpec(structSpec) {}

    TypeQualifier::TypeQualifier(const TokenPtr &tok) : tok(tok) {}

    FunctionSpecifier::FunctionSpecifier(const TokenPtr &tok) : tok(tok) {}

    StructOrUnionSpecifier::StructOrUnionSpecifier(const TokenPtr &structOrUnion,
                                                   const TokenPtr &id, const StructDeclarationListPtr &declList)
            : TypeSpecifier(StructOrUnionSpecifierPtr(this)), structOrUnion(structOrUnion), id(id), declList(declList) {}

    StructDeclaration::StructDeclaration(const SpecifierAndQualifierListPtr &specList,
                                         const StructDeclaratorListPtr &structDecrList) : specList(specList),
                                                                                          structDecrList(
                                                                                                  structDecrList) {}

    StructDeclarator::StructDeclarator(const DeclaratorPtr &decr, const ConstantExpressionPtr &constExpr) : decr(decr), constExpr(  constExpr) {}

    EnumSpecifier::EnumSpecifier(const TokenPtr &id, const EnumeratorListPtr &enumList) : id(id), enumList(enumList) {}

    Enumerator::Enumerator(const TokenPtr &enumConstant, const ConstantExpressionPtr &expr) : enumConstant(enumConstant), expr(expr) {}

    Declarator::Declarator(const PointerDeclPtr &pointer, const DirectDeclaratorPtr &directDecl) : pointer(pointer), directDecl(directDecl) {}

    Declarator::Declarator(const DirectDeclaratorPtr &directDecl) : directDecl(directDecl) {}

    PointerDecl::PointerDecl(const TypeQualifierListPtr &typeQualList, const PointerDeclPtr &pointerDecl)
            : typeQualList(typeQualList), pointerDecl(pointerDecl) {}

    PointerDecl::PointerDecl(const PointerDeclPtr &pointerDecl) : pointerDecl(pointerDecl) {}

    DirectDeclarator::DirectDeclarator(const TokenPtr &id) : id(id) { productionID = 0; }

    DirectDeclarator::DirectDeclarator(const DirectDeclaratorPtr &directDecl, bool isStatic,
                                       const AssignmentExpressionPtr &assignExpr,
                                       const TypeQualifierListPtr &typeQualList, int productionID) : directDecl(
            directDecl), isStatic(isStatic), assignExpr(assignExpr), typeQualList(typeQualList), productionID(
            productionID) {}

    DirectDeclarator::DirectDeclarator(const DirectDeclaratorPtr &directDecl, const TypeQualifierListPtr &typeQualList)
            : directDecl(directDecl), typeQualList(typeQualList) { hasPointer = true; productionID = 5; }

    DirectDeclarator::DirectDeclarator(const DirectDeclaratorPtr &directDecl, const WordTokenPtr &idList) : directDecl( directDecl), idList(idList) { productionID = 7; }

    DirectDeclarator::DirectDeclarator(const DirectDeclaratorPtr &directDecl, const ParameterTypeListPtr &paramList)
            : directDecl(directDecl), paramList(paramList) { productionID = 6; }

    DirectDeclarator::DirectDeclarator(const DeclaratorPtr &decl) : decl(decl) { productionID = 1; }

    ParameterTypeList::ParameterTypeList(const ParameterListPtr &paramList, bool isParameterVarible) : paramList( paramList), isParameterVarible(isParameterVarible) {}

    ParameterDecleartion::ParameterDecleartion(const DeclarationSpecifierPtr &declSpec, const DeclaratorPtr &decr)
            : declSpec(declSpec), decr(decr) {}

    ParameterDecleartion::ParameterDecleartion(const DeclarationSpecifierPtr &declSpec, const AbstractDeclaratorPtr &abstructDecr)
            : declSpec(declSpec), abstructDecr(abstructDecr) {}

    AbstractDeclarator::AbstractDeclarator(const PointerDeclPtr &pointerDecl, const DirectAbstractDeclaratorPtr &directAbstractDecr)
            : pointerDecl( pointerDecl), directAbstractDecr(directAbstractDecr) {}

    TypeName::TypeName(const SpecifierAndQualifierListPtr &specList, const AbstractDeclaratorPtr &abstructDecr)
            : specList(specList), abstructDecr(abstructDecr) {}

    DirectAbstractDeclarator::DirectAbstractDeclarator(const AbstractDeclaratorPtr &abstracrDecr) : abstracrDecr(
            abstracrDecr) { productionID = 0; }

    DirectAbstractDeclarator::DirectAbstractDeclarator(const DirectAbstractDeclaratorPtr &directAbstractDecr,
                                                       const AssignmentExpressionPtr &assignExpr,
                                                       const TypeQualifierListPtr &typeQualList,
                                                       bool  _isStatic, int _productionID)
            : directAbstractDecr(directAbstractDecr), assignExpr(assignExpr), typeQualList(typeQualList), isStatic(_isStatic)
    { productionID = _productionID; }

    DirectAbstractDeclarator::DirectAbstractDeclarator(const DirectAbstractDeclaratorPtr &directAbstractDecr,
                                                       const ParameterTypeListPtr &paramList) : directAbstractDecr(
            directAbstractDecr), paramList(paramList) { productionID = 5; }

    DirectAbstractDeclarator::DirectAbstractDeclarator(const DirectAbstractDeclaratorPtr &directAbstractDecr)
            : directAbstractDecr(directAbstractDecr) { hasPointer = true; productionID = 4; }

    Initializer::Initializer(const AssignmentExpressionPtr &assignExpr) : assignExpr(assignExpr) {}

    Initializer::Initializer(const InitializerListPtr &initList) : initList(initList) {}

    InitializerList::InitializerList(const InitializerPtr &init, const DesignationPtr &design,  const InitializerListPtr& initList) : init(init), design(design), initList(initList) {}

    Designation::Designation(const DesignatorListPtr &desList) : desList(desList) {}

    Designator::Designator(const ConstantExpressionPtr &constExpr) : constExpr(constExpr) {}

    Designator::Designator(const WordTokenPtr &id) : id(id) {}

    InitDeclarator::InitDeclarator(const DesignatorPtr &desOr, const InitializerPtr &init) : desOr(desOr), init(init) {}
}