#include "env.h"
#include "llvm/IR/Type.h"

namespace Miyuki::AST {
    GlobalScope GlobalScope::instance;
    IRBuilder<>& Builder = GlobalScope::instance.Builder;
    const size_t PointerSize = 4;

    LLVMContext & Miyuki::AST::getGlobalContext() {
        return GlobalScope::getInstance().context;
    }

    BasicBlock * Miyuki::AST::getCurrentBasicBlock() {
        return GlobalScope::getInstance().currentBasicBlock;
    }

    Function * Miyuki::AST::getCurrentFunction() {
        return GlobalScope::getInstance().currentFunction;
    }

    BasicBlock* Miyuki::AST::switchBasicBlock(string name) {
        BasicBlock* BB = BasicBlock::Create(getGlobalContext(), name, getCurrentFunction());
        Builder.SetInsertPoint(BB);
        GlobalScope::getInstance().currentBasicBlock = BB;
        return BB;
    }

    void Miyuki::AST::switchBasicBlock(BasicBlock * BB) {
        BasicBlock* BB = getCurrentBasicBlock();
        Function*   F = getCurrentFunction();
        assert(F && "Not in a function");
        BB->insertInto(F);
        Builder.SetInsertPoint(BB);
        GlobalScope::getInstance().currentBasicBlock = BB;
    }
}