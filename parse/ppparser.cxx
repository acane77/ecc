#include "ppparser.h"

namespace Miyuki::Parse {
    void PreprocessorParser::parse() {
        Lex::Token::flread = M_pplex->getSourceManager();
        next();
        //cout << (int)look->tag;
        while (look->isNot(-1)) {
            //cout << look->toString() << endl;
            next();
        }
        parseDone();
    }
}


