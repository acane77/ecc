#include "ast/extdefinitions.h"
#include "ast/env.h"
#include "ast/irutils.h"

namespace Miyuki::AST {

    TranslationUnit::TranslationUnit() {}

    ExternalDeclaration::ExternalDeclaration(const FunctionDefinitionPtr &funcDef) : funcDef(funcDef) {}

    ExternalDeclaration::ExternalDeclaration(const DeclarationPtr &decl) : decl(decl) {}

    FunctionDefinition::FunctionDefinition(const DeclarationSpecifierPtr &spec, const DeclaratorPtr &decr,
                                           const DeclarationListPtr &lst, const CompoundStatementPtr &stmt) :
            spec(spec), decr(decr), lst(lst), stmt(stmt) {}
	
	void TranslationUnit::gen() {
		for (ExternalDeclarationPtr extDecl: extDecls)
			extDecl->gen();
	}

	void ExternalDeclaration::gen() {
		if (decl)
			decl->gen();
		else if (funcDef)
			funcDef->gen();
	}

	void FunctionDefinition::gen() {
		if (lst) {
			reportError("K&R C-style function definition is not supported", getErrorToken());
			return;
		}

		getGlobalScope().isInFunctionDefPrototypePart = true;
		getCurrentScope()->enterScope();

		Type* T = spec->getType();
		T = decr->getType(T);
		FunctionType* FT = dyn_cast<FunctionType>(T);
		string FName = decr->getName();
		//cout << "Type Name: ";   T->dump();
		assert(FT && "not a function at all!!!");
		getGlobalScope().isInFunctionDefPrototypePart = false;

		Function* F = nullptr;

		// Insert a function
		F = Function::Create(FT, GlobalValue::LinkageTypes::ExternalLinkage, FName, TheModule);

		F = dyn_cast<Function>(TheModule->getOrInsertFunction(FName, FT));
		assert(F && "F == nullptr");
		
		// If F conflicted, there was already something named 'Name'.  If it has a
		// body, don't allow redefinition or reextern.
		if (F->getName() != FName) {
			// Delete the one we just made and get the existing one.
			F->eraseFromParent();
			F = TheModule->getFunction(FName);

			// If F already has a body, reject this.
			if (!F->empty()) {
				reportError("redefinition of function", getErrorToken());
				return;
			}

			// verify args
			auto FArgs = F->arg_begin();
			auto EArgs = FT->param_begin();

			// If F took a different number of args, reject.
			if (F->arg_size() != FT->params().size()) {
				reportError("redefinition of function with different # args", getErrorToken());
				return;
			}
			if (!(F->isVarArg() ^ FT->isVarArg())) {
				reportError("redefinition of function with different # args", getErrorToken());
				return;
			}

			// If type not compatible, reject
			for (int i=0; FArgs != F->arg_end() && EArgs != FT->param_end(); FArgs++, EArgs++, i++) {
				Type * FTy = FArgs->getType();
				Type * ETy = *EArgs;
				if (!TypeUtil::raiseType(FTy, ETy)) {
					reportError("parameter #{0} incompatible"_format(i), getErrorToken());
					return;
				}
			}
		}

		// set current function
		GlobalScope::getInstance().currentFunction = F;

		// Defaine basic blocks
		DefineBasicBlock(varinit);
		DefineBasicBlock(Entry);

		// Initial Basic Block
		getGlobalScope().functionInitBasicBlock = BB_varinit;

		// Insert this block before Entry
		BB_varinit->insertInto(F);
		
		// Allocate function paramters
		for (const std::pair<string, Type*>& e : getGlobalScope().functionParameters) {
			IdentifierPtr ID = make_shared<Identifier>(e.first, e.second, false,
				allocateIdentifier(e.first, e.second));
			if (!getCurrentScope()->getIndentifierFromThisScope(e.first))
				addIdentifier(ID);
			else
				// TODO:  set error token???
				reportError("redifinition of `{0}'"_format(e.first), nullptr);
		}

		// Save return type
		Type* retTy = F->getReturnType();
		getGlobalScope().functionReturnTy = retTy;
		if (!retTy->isVoidTy()) {
			// Allocate return value
			allocateIdentifier("retval", retTy);
		}

		setAsCurrentBasicBlock(BB_Entry);

		if (stmt)
			stmt->gen();

		GlobalScope::getInstance().currentFunction = nullptr;

		getCurrentScope()->leaveScope();

		// Insert a branch to BB_entry
		Builder.SetInsertPoint(BB_varinit);
		Builder.CreateBr(BB_Entry);

		// TODO:  leave function and return to global init function
	}

}