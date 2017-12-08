#include "common/flread.h"
#include "token.h"

namespace Miyuki::Lex {
    Miyuki::Common::SourceManagerPtr Token::flread = nullptr;
    int Token::startColumn = 0;

    const char * PunctuatorString[] = { "->", "++", "--", "<<", ">>", ">=", "<=", "==", "!=", "&&", "||", "...", "*=",
                                        "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "|=", "^=", "|=", ",", "#", "##", "<:", ":>", "<%", "%>", "%:", "%:%:"
    };

    string Token::toSourceLiteral() {
        if (tag == Tag::EndOfFile)
            return "EOF";
        else if (tag <= Tag::Property::OtherPunctuatorStart)
            return "{0}"_format((char)tag);
        else if (tag <= Tag::Property::PunctuatorEnd)
            return "{0}"_format(PunctuatorString[ tag - Tag::Property::OtherPunctuatorStart ]);
        return "Tag";
    }

    void Token::copyAdditionalInfo(const TokenPtr &tok) {
        startCol = tok->startCol;
        row = tok->row;
        column = tok->column;
        filenam = tok->filenam;
    }
}
