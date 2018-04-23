#include <gtest/gtest.h>
#include "ast/type.h"
#include "llvm/IR/LLVMContext.h"

TEST(TypeRaiseTest, TypeTest) {
    using namespace Miyuki::AST;
    llvm::LLVMContext ctx;
    EXPECT_TRUE(TypeUtil::raiseType(Type::getInt32Ty(ctx), Type::getInt32Ty(ctx))->isIntegerTy(32));
    EXPECT_TRUE(TypeUtil::raiseType(Type::getInt32Ty(ctx), Type::getInt64Ty(ctx))->isIntegerTy(64));
    EXPECT_TRUE(TypeUtil::raiseType(Type::getFloatTy(ctx), Type::getInt32Ty(ctx))->isFloatTy());
    EXPECT_TRUE(TypeUtil::raiseType(Type::getFloatTy(ctx), Type::getDoubleTy(ctx))->isDoubleTy());
    EXPECT_TRUE(TypeUtil::raiseType(Type::getDoubleTy(ctx), Type::getFloatTy(ctx))->isDoubleTy());
    EXPECT_TRUE(TypeUtil::raiseType(Type::getDoubleTy(ctx), Type::getInt32Ty(ctx))->isDoubleTy());
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}