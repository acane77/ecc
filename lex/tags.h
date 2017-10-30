#ifndef _MIYUKI_TAG_H
#define _MIYUKI_TAG_H

// tag.h  - defined tag used in lexer

namespace Miyuki::Lex::Tag {
    enum: int32_t {
        // Special
        EndOfFile = -1,

        // Other Punctuators
        PointerAccess = 128, Increase, Decrease, LeftShift, RightShift,
        GreaterThanEqual, LessThanEqual, Equal, NotEqual, And, Or, Ellipsis, MulpileAssign,
        DivideAssign, ModAssign, AddAssign, SubAssign, LeftShiftAssign,
        RightShiftAssign, BitwiseAndAssign, XorAssign, BitwiseOrAssign,
        Comma, DoubleSharp,
        LessThanColon, ColonGreaterThan, LessThanMod, ModGreaterThan,
        ModColon, ModColonDouble,

        // Keywords
        Alignof = 192, Auto, Asm, Break, Case, Char, Const, Continue, Default,
        Do, Double, Else, Enum, Extern, Float, For, Goto, If, Inline, Int,
        Long, Register, Restrict, Return, Short, Signed, Sizeof, Static,
        Struct, Switch, Typedef, Union, Unsigned, Void, Volatile, While,
        KAlignas, KAtomic, KBool, KComplex, KGeneric, KImaginary, KNoreturn,
        KStatic_assert, KThread_local,

        // Types
        ID              = 1 << 8,
        Integer         = 1 << 9,
        Floating        = 1 << 10,
        Enumeration     = 1 << 11,
        Character       = 1 << 12,
        StringLiteral   = 1 << 13,
        HeaderName      = 1 << 14,
        PPNumber        = 1 << 15,
        PPLiteral       = 1 << 16,
        Number = Integer | Floating,
        Constant = Integer | Floating | Enumeration,
        Identifier = ID | Enumeration,
        Punctuator = -2
    };

    enum Property : uint32_t {
        PunctuatorStart = 0,
        OtherPunctuatorStart = 128,
        PunctuatorEnd = 191,
        KeywordStart = 192,
        KeywordEnd = 255,
    };
}

#endif
