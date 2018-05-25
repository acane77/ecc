#include "statements.h"
#include "ast/statements.h"
#include "ast/env.h"
#include "ast/irutils.h"
#include "ast/value.h"
#include <stack>

namespace Miyuki::AST {

    LabeledStatement::LabeledStatement(const StatementPtr &stmt, const WordTokenPtr &id) : stmt(stmt), id(id) {}

    LabeledStatement::LabeledStatement(const StatementPtr &stmt) : stmt(stmt) { isDefault = true; }

    LabeledStatement::LabeledStatement(const StatementPtr &stmt, const ConstantExpressionPtr &constExpr) : stmt(stmt), constExpr(constExpr) {}

    CompoundStatement::CompoundStatement(const BlockItemListPtr &blkItemLst) : blkItemLst(blkItemLst) {}

    BlockItem::BlockItem(const DeclarationPtr &decl) : decl(decl) {}

    BlockItem::BlockItem(const StatementPtr &stmt) : stmt(stmt) {}

    ExpressionStatement::ExpressionStatement(const ExpressionPtr &expr) : expr(expr) {}

    If::If(const ExpressionPtr &condExpr, const StatementPtr &stmt, const ElsePtr &elseStmt) : condExpr(condExpr), stmt(stmt), elseStmt(elseStmt) {}

    Else::Else(const StatementPtr &stmt) : stmt(stmt) {}

    Switch::Switch(const ExpressionPtr &expr, const StatementPtr &stmt) : expr(expr), stmt(stmt) {}

    While::While(const ExpressionPtr &condExpr, const StatementPtr &stmt) : condExpr(condExpr), stmt(stmt) {}

    DoWhile::DoWhile(const ExpressionPtr &condExpr, const StatementPtr &stmt) : condExpr(condExpr), stmt(stmt) {}

    For::For(const DeclarationPtr &decl, const ExpressionPtr &init, const ExpressionPtr &cond, const ExpressionPtr &inc,
             const StatementPtr &stmt) : decl(decl), init(init), cond(cond), inc(inc), stmt(stmt) {}

    For::For(const ExpressionPtr &init, const ExpressionPtr &cond, const ExpressionPtr &inc, const StatementPtr &stmt)
            : init(init), cond(cond), inc(inc), stmt(stmt) {}

    Goto::Goto(const TokenPtr &id) : id(id) {}

    Return::Return(const ExpressionPtr &expr) : expr(expr) {}

	/// --------  Code Generation --------

#define EnterAnotherScope() getCurrentScope()->enterScope()
#define LeaveThisScope()    getCurrentScope()->leaveScope()

	struct LoopOrSwitch {
		BasicBlock* start;
		BasicBlock* end;
		bool isLoop;
		int numCases = 0;

		LoopOrSwitch(BasicBlock* s, BasicBlock* e, bool i) {
			start = s;
			end = e;
			isLoop = i;
		}
	};

	std::deque<LoopOrSwitch> LoopOrSwitchStack;
	std::map<string, BasicBlock*> labels;

	void enterALoopOrSwitch(BasicBlock* BBStart, BasicBlock* BBEnd, bool isLoop = true) {
		EnterAnotherScope();
		LoopOrSwitchStack.push_back(LoopOrSwitch(BBStart, BBEnd, isLoop));
	}

	void leaveThisLoopOrSwitch() {
		LeaveThisScope();
		LoopOrSwitchStack.pop_back();
	}

	int getNumCasesIfTheLastIsSwitch() {
		if (LoopOrSwitchStack.size() == 0)
			return -1;
		LoopOrSwitch LS = LoopOrSwitchStack.back();
		if (LS.isLoop)
			return -1;
		return LS.numCases;
	}

	BasicBlock* GetEndOfThisLoopOrStack() {
		if (LoopOrSwitchStack.size() == 0)
			return nullptr;
		return LoopOrSwitchStack.back().end;
	}

	BasicBlock* GetStartOfLastLoop() {
		for (deque<LoopOrSwitch>::reverse_iterator it = LoopOrSwitchStack.rbegin();
			it != LoopOrSwitchStack.rend(); ++it) {
			if (it->isLoop)
				return it->start;
		}
		return nullptr;
	}

#define EnterThisLoop enterALoopOrSwitch
#define LeaveThisLoop leaveThisLoopOrSwitch

	void CompoundStatement::gen() {
		getCurrentScope()->enterScope();
		
		if (blkItemLst) {
			for (BlockItemPtr BI : *blkItemLst)
				BI->gen();
		}
			
		getCurrentScope()->leaveScope();
	}

	void BlockItem::gen() {
		if (decl)
			decl->gen();
		else if (stmt)
			stmt->gen();
		else
			assert(!"both decl and stmt are nullptr");
	}

	void ExpressionStatement::gen() {
		if (expr)
			expr->gen();
	}

	void If::gen() {
		DefineBasicBlock(True);
		DefineBasicBlock(False);
		DefineBasicBlock(EndIf);

		condExpr->gen();
		Value * cond = condExpr->getAddr();
		cond = createComparationToZero(cond);
		Builder.CreateCondBr(cond, BB_True, BB_False);

		SetInsertBlock(BB_True);
		assert(stmt && "stmt == nullptr");
		stmt->gen();
		Builder.CreateBr(BB_EndIf);

		SetInsertBlock(BB_False);
		if (elseStmt)
			elseStmt->gen();
		Builder.CreateBr(BB_EndIf);

		SetInsertBlock(BB_EndIf);
	}

	void Else::gen() {
		stmt->gen();
	}

	void While::gen() {
		DefineBasicBlock(WhileCond);
		DefineBasicBlock(WhileBody);
		DefineBasicBlock(EndWhile);
		
		EnterThisLoop(BB_WhileCond, BB_EndWhile);

		Builder.CreateBr(BB_WhileBody);
		
		SetInsertBlock(BB_WhileCond);
		condExpr->gen();
		Value* cond = condExpr->getAddr();
		cond = createComparationToZero(cond);
		Builder.CreateCondBr(cond, BB_WhileBody, BB_EndWhile);

		SetInsertBlock(BB_WhileBody);
		stmt->gen();
		Builder.CreateBr(BB_WhileCond);

		SetInsertBlock(BB_EndWhile);
		LeaveThisLoop();
	}

	void DoWhile::gen() {
		DefineBasicBlock(WhileBody);
		DefineBasicBlock(EndWhile);

		EnterThisLoop(BB_WhileBody, BB_EndWhile);

		Builder.CreateBr(BB_WhileBody);

		SetInsertBlock(BB_WhileBody);
		stmt->gen();
		// condition
		Value* cond = condExpr->getAddr();
		cond = createComparationToZero(cond);
		Builder.CreateCondBr(cond, BB_WhileBody, BB_EndWhile);

		SetInsertBlock(BB_EndWhile);
		LeaveThisLoop();
	}

	void For::gen() {
		DefineBasicBlock(Init);
		DefineBasicBlock(Cond);
		DefineBasicBlock(Body);
		DefineBasicBlock(End);
		
		EnterThisLoop(BB_Cond, BB_End);

		if (decl)
			decl->gen();
		else if (init)
			init->gen();
		Builder.CreateBr(BB_Cond);

		SetInsertBlock(BB_Cond);
		if (cond) {
			cond->gen();
			Value* c = cond->getAddr();
			c = createComparationToZero(c);
			Builder.CreateCondBr(c, BB_Body, BB_End);
		}
		else {
			Builder.CreateBr(BB_Body);
		}

		SetInsertBlock(BB_Body);
		stmt->gen();
		inc->gen();
		Builder.CreateBr(BB_Cond);

		SetInsertBlock(BB_End);
		LeaveThisLoop();
	}


	void Return::gen() {
		DefineBasicBlock(Ret);
		if (expr) {
			expr->gen();
			Builder.CreateRet(expr->getAddr());
		}
		else {
			Builder.CreateRetVoid();
		}
		SetInsertBlock(BB_Ret);
	}

	void Continue::gen() {
		BasicBlock * Start = GetStartOfLastLoop();
		if (Start == nullptr) {
			reportError("invalid 'continue' outside loop", getErrorToken());
			return;
		}

		DefineBasicBlock(AfterContinue);
		Builder.CreateBr(Start);
		SetInsertBlock(BB_AfterContinue);
	}

	void Break::gen() {
		BasicBlock * End = GetEndOfThisLoopOrStack();
		if (End == nullptr) {
			reportError("invalid 'break' outside loop or switch", getErrorToken());
			return;
		}

		DefineBasicBlock(AfterBreak);
		Builder.CreateBr(End);
		SetInsertBlock(BB_AfterBreak);
	}

	void LabeledStatement::gen() {
		// identifier:
		if (id) {
			DefineBasicBlock(GotoLbl);
			Builder.CreateBr(BB_GotoLbl);
			string name = static_pointer_cast<WordToken>(id)->name;
			labels[name] = BB_GotoLbl;
			SetInsertBlock(BB_GotoLbl);
		}
		// case xx:
		else if (constExpr) {
			int numCases = getNumCasesIfTheLastIsSwitch();
			if (numCases == -1) {
				reportError("invalid case outside switch", getErrorToken());
				return;
			}
			LoopOrSwitchStack.back().numCases++;
			constExpr->eval();
			/// TODO: [TODO:1]
			if (constExpr->IsCalculated()) {
				reportError("cases must be constant", getErrorToken());
				return;
			}

			Value * V = ValueFactory::build(constExpr->getCalculatedToken());
			DefineBasicBlock(Case);
			/// TODO: ??????  numCases 参数的意思
			Builder.CreateSwitch(V, BB_Case, numCases + 1);
		}
		// default:
		else if (isDefault) {
			int numCases = getNumCasesIfTheLastIsSwitch();
			if (numCases == -1) {
				reportError("invalid default outside switch", getErrorToken());
				return;
			}
			/// TODO: implement it
		}
	}

	void Goto::gen() {
		/// TODO: restrictions of GOTO
		auto it = labels.find(static_pointer_cast<WordToken>(id)->name);
		DefineBasicBlock(AfterGoto);
		Builder.CreateBr(it->second);
		SetInsertBlock(BB_AfterGoto);
	}
}