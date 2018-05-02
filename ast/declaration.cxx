#include "declaration.h"
#include "ast/env.h"

namespace Miyuki::AST {

    Declaration::Declaration(const DeclarationSpecifierPtr &decSpec, const InitDeclaratorListPtr &initDeclList)
            : decSpec(decSpec), initDeclList(initDeclList) {}

    DeclarationSpecifier::DeclarationSpecifier(const SpecifierAndQualifierPtr &spec,
                                               const DeclarationSpecifierPtr &decSpec) : spec(spec), decSpec(decSpec) {}

    StorageClassSpecifier::StorageClassSpecifier(const TokenPtr &tok) : tok(tok) {}

    TypeSpecifier::TypeSpecifier(const TokenPtr &tok) : tok(tok) {}

    TypeSpecifier::TypeSpecifier() {}

    TypeQualifier::TypeQualifier(const TokenPtr &tok) : tok(tok) {}

    FunctionSpecifier::FunctionSpecifier(const TokenPtr &tok) : tok(tok) {}

    StructOrUnionSpecifier::StructOrUnionSpecifier(const TokenPtr &structOrUnion,
                                                   const TokenPtr &id, const StructDeclarationListPtr &declList)
            : structOrUnion(structOrUnion), id(id), declList(declList) {}

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
            productionID) {
        isArrayDeclarator = true;
    }

    DirectDeclarator::DirectDeclarator(const DirectDeclaratorPtr &directDecl, const TypeQualifierListPtr &typeQualList)
            : directDecl(directDecl), typeQualList(typeQualList) { 
        hasPointer = true; productionID = 5;
        isArrayDeclarator = true;
    }

    DirectDeclarator::DirectDeclarator(const DirectDeclaratorPtr &directDecl, const WordTokenListPtr &idList) : directDecl( directDecl), idList(idList) { 
        productionID = 7;
        isOldStyleFunctionPrototypeDeclaration = true;
    }

    DirectDeclarator::DirectDeclarator(const DirectDeclaratorPtr &directDecl, const ParameterTypeListPtr &paramList)
            : directDecl(directDecl), paramList(paramList) { 
        productionID = 6;
        isFunctionPrototypeDeclaration = true;
    }

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
    { 
        productionID = _productionID;
        isArrayDeclarator = true;
    }

    DirectAbstractDeclarator::DirectAbstractDeclarator(const DirectAbstractDeclaratorPtr &directAbstractDecr,
                                                       const ParameterTypeListPtr &paramList) : directAbstractDecr(
            directAbstractDecr), paramList(paramList) { 
        productionID = 5;
        isFunctionPrototypeDeclaration = true;
    }

    DirectAbstractDeclarator::DirectAbstractDeclarator(const DirectAbstractDeclaratorPtr &directAbstractDecr)
            : directAbstractDecr(directAbstractDecr) { 
        hasPointer = true; productionID = 4;
        isArrayDeclarator = true;
    }

    Initializer::Initializer(const AssignmentExpressionPtr &assignExpr) : assignExpr(assignExpr) {}

    Initializer::Initializer(const InitializerListPtr &initList) : initList(initList) {}

    InitializerList::InitializerList(const InitializerPtr &init, const DesignationPtr &design,  const InitializerListPtr& initList) : init(init), design(design), initList(initList) {}

    Designation::Designation(const DesignatorListPtr &desList) : desList(desList) {}

    Designator::Designator(const ConstantExpressionPtr &constExpr) : constExpr(constExpr) {}

    Designator::Designator(const WordTokenPtr &id) : id(id) {}

    InitDeclarator::InitDeclarator(const DeclaratorPtr &desOr, const InitializerPtr &init) : desOr(desOr), init(init) {}

    /// Reprot error
    void reportError(string errMsg, TokenPtr tok) {
        cout << "find error :" << errMsg << endl;
    }

    /// Type Related Function
    // Generate type information including storage class, specifiers and qualifiers and its return type
    PackedTypeInformationPtr getMemberTypeFromSpecifierAndQualifierList(SpecifierAndQualifierListPtr L) {
        StorageClass storageClass;
        TypeQualifierPtr typeQualifier; // type qualifier is attach to a Type object
        FunctionSpecifierFlag functionSpec;
        bool hasFunctionSpec = false;
        bool hasRestrictAttribute = false;
        bool hasVolatileAttribute = false;
        TypePtr _ret = nullptr;
        TokenPtr lastTok = nullptr;
        TypeQualifierFlag qualifier;

        for (const SpecifierAndQualifierPtr& SQ : *L) {
            if (SQ->isStorageClassSpecifier()) {
                lastTok = static_pointer_cast<StorageClassSpecifier>(SQ)->tok;
                if (storageClass.flag & SQ->getStorageClass().flag) {
                    reportError("duplicate storage class here", lastTok);
                }
                storageClass.flag |= SQ->getStorageClass().flag;
                continue;
            }
            else if (SQ->isTypeQualifier()) {
                lastTok = static_pointer_cast<TypeQualifier>(SQ)->tok;
                if (typeQualifier) {
                    reportError("duplicate `const' qualifier here", lastTok);
                    continue;
                }
                typeQualifier = static_pointer_cast<TypeQualifier>(SQ);
                typeQualifier->getTypeQualifier(qualifier);
                if (qualifier.isConst)
                    continue;
                else if (qualifier.isRestrict) {
                    if (hasRestrictAttribute)
                        reportError("duplicate `restrict' qualifier here", lastTok);
                    hasRestrictAttribute = true;
                }
                else if (qualifier.isVolatile) {
                    if (hasVolatileAttribute)
                        reportError("duplicate `volatile' qualifier here", lastTok);
                    hasVolatileAttribute = true;
                }
                else if (qualifier.isAtomic) {
                    /// DO NOTHING due to unsupport of atomic 
                }

                // only const type qualifier stores TypeQualifier object
                typeQualifier = nullptr;
                continue;
            }
            else if (SQ->isFunctionSpecifier()) {
                hasFunctionSpec = true;
                FunctionSpecifierPtr spec = static_pointer_cast<FunctionSpecifier>(SQ);
                lastTok = spec->tok;
                if (functionSpec.flag & spec->getFunctionSpecifier().flag)
                    reportError("duplicate function specifier here", lastTok);
                functionSpec.flag |= spec->getFunctionSpecifier().flag;
                continue;
            }
            // else is type specifier
            else {
                if (_ret) {
                    reportError("duplicate tye qualifier here", static_pointer_cast<TypeSpecifier>(SQ)->tok);
                    continue;
                }
                _ret = SQ->getType();
            }
        }

        if (!_ret) {
            reportError("No type specified", lastTok);
            return nullptr;
        }

        return make_shared<PackedTypeInformation>(_ret, storageClass, functionSpec, qualifier);
    }

    /// * Type Specifier
    TypePtr TypeSpecifier::getType() {
        // QUESTION: only for typedef names?
        if (!tok) {
            assert(0 && "invalid typedef-name");
            return nullptr;
        }
        auto it = GlobalScope::getInstance().typedefs.find( static_pointer_cast<WordToken>(tok)->name );
        if (it == GlobalScope::getInstance().typedefs.end())
            assert(0 && "invalid typedef name");
        return it->second; //TODO: reconize typedef name in typedefs
    }

    ///   * Storage Class Specifier
    StorageClass Miyuki::AST::StorageClassSpecifier::getStorageClass() {
        StorageClass flag;
        switch (tok->tag) {
        case Tag::Typedef:   flag.hasTypedef = true; break;
        case Tag::Extern:    flag.isExtern = true;   break;
        case Tag::Static:    flag.isStatic = true;   break;
        case Tag::KThread_local: flag.isThreadLocal = true; break;
        case Tag::Auto:      flag.isAuto = true; break;
        case Tag::Register:  flag.isRegister = true; break;
        default: assert(false && "invalid storage class.");
        }
        return move(flag);
    }

    ////  * Type qualifier
    void Miyuki::AST::TypeQualifier::getTypeQualifier(TypeQualifierFlag & flag) {
        switch (tok->tag) {
        case Tag::Const: flag.isConst = true; return;
        case Tag::Restrict: flag.isConst = true; return;
        case Tag::Volatile: flag.isRestrict = true; return;
        case Tag::KAtomic:  flag.isAtomic = true; return;
        }
        assert(false && "invalid type-qualifier");
    }

    /// * Function Specifier
    FunctionSpecifierFlag Miyuki::AST::FunctionSpecifier::getFunctionSpecifier() {
        FunctionSpecifierFlag FS;
        switch (tok->tag) {
        case Tag::KNoreturn: FS.isNoReturn = true; return FS;
        case Tag::Inline: FS.isInline = true; return FS;
        }
        assert(false && "invalid function-specifier");
        return FS;
    }

    // StructOrUnion
    TypePtr StructOrUnionSpecifier::getType() {
        // TODO: implement union type
        return getStructTy();
    }

    TypePtr StructOrUnionSpecifier::getUnionTy()  {
        assert(false && "union type is not supported yet.");
        return nullptr;
    }

    TypePtr StructOrUnionSpecifier::getStructTy() {
        IndexedTypeInformation::IndexType memberIndex = 0;
        static int unnamedStructID = 0;
        string structName = id ?
            static_pointer_cast<WordToken>(id)->name :
            "<unnamed-struct-{0}>"_format(unnamedStructID++);
        StructTyPtr TY = nullptr;
        if (TY = StructTy::get(structName)) {
            reportError("struct {0} redefined.", id);
            return TY->type;
        }
        IndexedTypeMapPtr TM = make_shared<IndexedTypeMap>();
        vector<Type*> eles;
        for (const StructDeclarationPtr& decl : *declList) {
            decl->getMember(eles, TM, memberIndex);
        }
        TY = make_shared<StructTy>(TM, structName, eles);
        StructTy::saveStruct(TY);
        return TY->type;
    }

    string StructDeclarator::getName() {
        return decr->getName();
    }

    string Declarator::getName() {
        return directDecl->getName();
    }

    string DirectDeclarator::getName() {
        if (id)
            return static_pointer_cast<WordToken>(id)->name;
        else if (decl)
            return decl->getName();
        else if (directDecl)
            return directDecl->getName();
        assert(false && "is not a vivid direct-declarator");
        return string("<no-name>");
    }

    void StructDeclaration::getMember(vector<Type*>& memList, const IndexedTypeMapPtr& typeMap, IndexedTypeInformation::IndexType& index) {
        PackedTypeInformationPtr typeInfo = getMemberTypeFromSpecifierAndQualifierList(specList);
        static uint32_t unamedMemberID = 1;
        // if has no member names
        if (structDecrList) {
            // add members with no name
            //StructDef::StructMemberPtr member = make_shared<StructDef::StructMember>("<unnamed_member_{0}>"_format(unamedMemberID), typeInfo);
            memList.push_back(typeInfo->type);
            IndexedTypeInformationPtr IT = make_shared<IndexedTypeInformation>(*typeInfo, index++);
            (*typeMap)["<unnamed-member-{0}>"_format(unamedMemberID)] = IT;
        }

        /// reconize members
        for (const StructDeclaratorPtr& decl : *structDecrList) {
            // add members
            IndexedTypeInformationPtr TI = make_shared<IndexedTypeInformation>(*typeInfo, index++);
            TI->type = decl->getType(TI->type);
            string memName = decl->getName();
            memList.push_back(TI->type);
            (*typeMap)[memName] = TI;
        }
    }

    // Struct declarator
    TypePtr StructDeclarator::getType(TypePtr basetype) {
        return decr->getType(basetype);
    }

    // Declarator
    TypePtr Declarator::getType(TypePtr baseType) {
        TypePtr type;
        if (pointer) {
            type = pointer->getType(baseType);
        }
        assert(directDecl && "directDecl == nullptr");
        return directDecl->getType(baseType);
    }

    // Pointer
    TypePtr PointerDecl::getType(TypePtr baseType) {
        if (typeQualList) {
            for (const TypeQualifierPtr& Q : *typeQualList) {
                TypeQualifierFlag qual;
                Q->getTypeQualifier(qual);
                if (qual.isConst) {
                    //baseType = baseType->getConstType();
                    /// TODO: Give qualifier to caller
                }
            }
        }
        baseType = PointerType::get(baseType, 0);
        if (pointerDecl)
            baseType = pointerDecl->getType(baseType);
        return baseType;
    }

    TypePtr DirectDeclarator::getType(TypePtr baseType) {
        // if is ID token
        if (id) {
            return baseType;
        }
        // if is declarator
        else if (decl) {
            return decl->getType(baseType);
        }
        // is-array
        else if (isArrayDeclarator) {
            /// NOTE: ignore static and type-qualifier
            /// In order to simplify, I completely process [ expression ] like a pointer,
            /// and varible-length array is not allowed
            return PointerType::get(directDecl->getType(baseType), 0);
        }
        // is function definition
        else if (isFunctionPrototypeDeclaration) {
            vector<TypePtr> params;
            paramList->generateTypeList(params);
            FunctionType* FT = FunctionType::get(baseType, params, paramList->isParameterVarible);
            return FT;
        }
        else if (isOldStyleFunctionPrototypeDeclaration) {
            reportError("K&R C Grammar is not supported.", nullptr);
        }
    }

    void ParameterTypeList::generateTypeList(vector<TypePtr>& list) {
        for (const ParameterDecleartionPtr& PD : *paramList) {
            PackedTypeInformationPtr typeInfo = PD->getTypeInfo();
            /// TODO: add function parameter here to ID list?
            list.push_back(typeInfo->type);
        }
    }

    // ParameterDecleartion
    PackedTypeInformationPtr Miyuki::AST::ParameterDecleartion::getTypeInfo() {
        SpecifierAndQualifierListPtr lst = declSpec->generateSpecifierQualifierList();
        PackedTypeInformationPtr typeInfo = getMemberTypeFromSpecifierAndQualifierList(lst);
        if (decr) {
            typeInfo->type = decr->getType(typeInfo->type);
        }
        else if (abstructDecr) {
            typeInfo->type = abstructDecr->getType(typeInfo->type);
        }
        else assert(false && "decr == nullptr && abstructDecr == nullptr");
        return typeInfo;
    }

    // DeclarationSpecifier
    SpecifierAndQualifierListPtr Miyuki::AST::DeclarationSpecifier::generateSpecifierQualifierList() {
        SpecifierAndQualifierListPtr lst = make_shared<SpecifierAndQualifierList>();
        _genSpecQualList(lst);
        return lst;
    }

    void Miyuki::AST::DeclarationSpecifier::_genSpecQualList(SpecifierAndQualifierListPtr lst) {
        lst->push_back(spec);
        if (decSpec)
            decSpec->_genSpecQualList(lst);
    }

    // AbstractDeclarator
    TypePtr Miyuki::AST::AbstractDeclarator::getType(TypePtr baseType) {
        TypePtr type;
        if (pointerDecl) {
            type = pointerDecl->getType(baseType);
        }
        assert(directAbstractDecr && "directDecl == nullptr");
        return directAbstractDecr->getType(baseType);
    }

    // DirectAbstractDeclarator
    TypePtr Miyuki::AST::DirectAbstractDeclarator::getType(TypePtr baseType) {
        // if is declarator
        if (abstracrDecr) {
            return abstracrDecr->getType(baseType);
        }
        // is-array
        else if (isArrayDeclarator) {
            /// NOTE: ignore static and type-qualifier
            /// In order to simplify, I completely process [ expression ] like a pointer,
            /// and varible-length array is not allowed
            return PointerType::get(directAbstractDecr->getType(baseType), 0);
        }
        // is function definition
        else if (isFunctionPrototypeDeclaration) {
            vector<TypePtr> params;
            paramList->generateTypeList(params);
            FunctionType* FT = FunctionType::get(baseType, params, paramList->isParameterVarible);
            return FT;
        }
        else if (isOldStyleFunctionPrototypeDeclaration) {
            reportError("K&R C Grammar is not supported.", nullptr);
        }
    }

    // TypeName
    PackedTypeInformationPtr Miyuki::AST::TypeName::getType() {
        PackedTypeInformationPtr typeInfo = getMemberTypeFromSpecifierAndQualifierList(specList);
        typeInfo->type = abstructDecr->getType(typeInfo->type);
        return typeInfo;
    }

    // Initiallizer List
    TypePtr Initializer::getType() {
        if (assignExpr)
            return assignExpr->getSymbolType();
        return initList->getType();
    }
}