#include "ast/extdefinitions.h"
#include "ast/env.h"
#include "ast/irutils.h"

namespace Miyuki::AST {

    TranslationUnit::TranslationUnit(const ExternalDeclarationPtr &extDecl, const TranslationUnitPtr &unit) :
            extDecl(extDecl), unit(unit) {}

    ExternalDeclaration::ExternalDeclaration(const FunctionDefinitionPtr &funcDef) : funcDef(funcDef) {}

    ExternalDeclaration::ExternalDeclaration(const DeclarationPtr &decl) : decl(decl) {}

    FunctionDefinition::FunctionDefinition(const DeclarationSpecifierPtr &spec, const DeclaratorPtr &decr,
                                           const DeclarationListPtr &lst, const CompoundStatementPtr &stmt) :
            spec(spec), decr(decr), lst(lst), stmt(stmt) {}
	
	void TranslationUnit::gen() {
		extDecl->gen();
		if (unit)
			unit->gen();
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

		FunctionType* FT = dyn_cast<FunctionType>(decr->getType(spec->getType()));
		string FName = decr->getName();
		
		assert(FT && "not a function at all!!!");
		getGlobalScope().isInFunctionDefPrototypePart = false;

		Function* F = nullptr;
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

		F = Function::Create(FT, GlobalValue::LinkageTypes::ExternalLinkage, FName, TheModule);
		GlobalScope::getInstance().currentFunction = F;

		if (stmt)
			stmt->gen();

		getCurrentScope()->leaveScope();
	}

}