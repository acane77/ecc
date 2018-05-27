#include "ast/util.h"

namespace Miyuki::AST {

	Value * _CreateInst_typeConv(Value * v1, Type * t2) {
		Type* t1 = v1->getType();
		assert(CastInst::isCastable(t1, t2) && "not castable");

		Instruction::CastOps ops = CastInst::getCastOpcode(v1, true, t2, true);
		
		return Builder.CreateCast(ops, v1, t2, "ty.cast");
	}

	bool CodeGenUtils::CreateTypeConversion(Value *& v1, Value *& v2) {
		Type* t1 = v1->getType(), *t2 = v2->getType();
		
		// check if they are same ty
		assert(t1 != t2 && "they are the same ty");

		if (!CastInst::isCastable(t1, t2))
			return false;

		Type* Ty = TypeUtil::raiseType(t1, t2);

		if (t1 != Ty)
			v1 = _CreateInst_typeConv(v1, t2);
		else if (t2 != Ty)
			v2 = _CreateInst_typeConv(v2, t2);
		else assert(!"nothing to cast");

		return true;
	}

}