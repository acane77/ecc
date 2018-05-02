#include "ast/statements.h"

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
}