#ifndef _MIYUKI_STATEMENTS_H
#define _MIYUKI_STATEMENTS_H

#include "ast/symbols.h"
#include "ast/expression.h"
#include "ast/declaration.h"

// Defines symbols for statements

namespace Miyuki::AST {
    
    DEFINE_SHARED_PTR(Statement)
    DEFINE_SHARED_PTR(LabeledStatement)
    DEFINE_SHARED_PTR(CompoundStatement)
    DEFINE_SHARED_PTR(BlockItem)
    DEFINE_LIST(BlockItem)
    DEFINE_SHARED_PTR(ExpressionStatement)
    DEFINE_SHARED_PTR(If)
    DEFINE_SHARED_PTR(Else)
    DEFINE_SHARED_PTR(Switch)
    DEFINE_SHARED_PTR(While)
    DEFINE_SHARED_PTR(DoWhile)
    DEFINE_SHARED_PTR(For)
    DEFINE_SHARED_PTR(Goto)
    DEFINE_SHARED_PTR(Continue)
    DEFINE_SHARED_PTR(Break)
    DEFINE_SHARED_PTR(Return)

    typedef int Label;

    class Statement : public Symbol {
    public:
        Label lblNext;
        virtual void gen() { assert( false && "not implemented" ); }
        virtual int getKind() override { return Kind::statement; }
    };

    class LabeledStatement: public Statement{
    public:
        StatementPtr stmt = nullptr;
        
        WordTokenPtr id = nullptr;
        // OR
        ConstantExpressionPtr constExpr = nullptr;
        // OR
        bool         isDefault = false;

        LabeledStatement(const StatementPtr &stmt, const WordTokenPtr &id);
        LabeledStatement(const StatementPtr &stmt, const ConstantExpressionPtr &constExpr);
        explicit LabeledStatement(const StatementPtr &stmt);

		virtual void gen() override;
        virtual int getKind() override { return Kind::labeledStatement; }
    };

    class CompoundStatement : public Statement {
    public:
        BlockItemListPtr blkItemLst;

        CompoundStatement(const BlockItemListPtr &blkItemLst);

		virtual void gen() override;
        virtual int getKind() override { return Kind::compoundStatement; }
    };

    class BlockItem : public Statement {
    public:
        DeclarationPtr  decl = nullptr;
        // OR
        StatementPtr    stmt = nullptr;

        explicit BlockItem(const StatementPtr &stmt);
        explicit BlockItem(const DeclarationPtr &decl);

		virtual void gen() override;
        virtual int getKind() override { return Kind::blockItem; }
    };

    class ExpressionStatement : public Statement {
    public:
        ExpressionPtr   expr = nullptr;

        explicit ExpressionStatement(const ExpressionPtr &expr);

		virtual void gen() override;
        virtual int getKind() override { return Kind::expressionStatement; }
    };

    // Selection statements
    class If : public Statement {
    public:
        ExpressionPtr   condExpr = nullptr;
        StatementPtr    stmt = nullptr;
        ElsePtr         elseStmt = nullptr;

        If(const ExpressionPtr &condExpr, const StatementPtr &stmt, const ElsePtr &elseStmt = nullptr);

		virtual void gen() override;
        virtual int getKind() override { return Kind::_if; }
    };

    class Else : public Statement {
    public:
        StatementPtr    stmt;

        explicit Else(const StatementPtr &stmt);

		virtual void gen() override;
        virtual int getKind() override { return Kind::_else; }
    };

    class Switch : public Statement {
    public:
        ExpressionPtr   expr;
        StatementPtr    stmt;

        Switch(const ExpressionPtr &expr, const StatementPtr &stmt);

        virtual void gen() override{}
        virtual int getKind() override { return Kind::_switch; }
    };

    // Iteration statements
    class While : public Statement {
    public:
        ExpressionPtr   condExpr;
        StatementPtr    stmt;

        While(const ExpressionPtr &condExpr, const StatementPtr &stmt);

		virtual void gen() override;
        virtual int getKind() override { return Kind::_while; }
    };

    class DoWhile : public Statement {
    public:
        ExpressionPtr   condExpr;
        StatementPtr    stmt;

        DoWhile(const ExpressionPtr &condExpr, const StatementPtr &stmt);

		virtual void gen() override;
        virtual int getKind() override { return Kind::doWile; }
    };

    class For : public Statement {
    public:
        DeclarationPtr decl = nullptr;
        ExpressionPtr  init = nullptr; //initial expression
        ExpressionPtr  cond = nullptr; //conditional expression
        ExpressionPtr  inc  = nullptr;  //increment expression
        StatementPtr   stmt = nullptr;

        For(const DeclarationPtr &decl, const ExpressionPtr &init, const ExpressionPtr &cond, const ExpressionPtr &inc,
            const StatementPtr &stmt);
        For(const ExpressionPtr &init, const ExpressionPtr &cond, const ExpressionPtr &inc, const StatementPtr &stmt);

		virtual void gen() override;
        virtual int getKind() override { return Kind::_for; }
    };

    // jump statements
    class Goto : public Statement {
    public:
        TokenPtr id;

        explicit Goto(const TokenPtr &id);

		virtual void gen() override;
        virtual int getKind() override { return Kind::_goto; }
    };

    class Continue : public Statement {
    public:

        virtual void gen() override;
        virtual int getKind() override { return Kind::_continue; }
    };

    class Break : public Statement {
    public:

		virtual void gen() override;
        virtual int getKind() override { return Kind::_break; }
    };

    class Return : public Statement {
    public:
        ExpressionPtr expr = nullptr;

        explicit Return(const ExpressionPtr &expr);

		virtual void gen() override;
        virtual int getKind() override { return Kind::_return; }
    };
}

#endif