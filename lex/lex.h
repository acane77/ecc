#ifndef _MIYUKI_LEXER_H
#define _MIYUKI_LEXER_H

#include "include.h"
#include "common/ptrdef.h"
#include "common/srcmgr.h"
#include "lex/token.h"
#include "interfaces.h"

namespace Miyuki::Lex {
    using namespace Miyuki::Common;

    DEFINE_SHARED_PTR(Lexer)

    class Lexer {
    protected:
        SourceManagerPtr M_sm;
        int peak;

        // Get character value with a slash
        char _getCharFromSlash();
        // Skip commemt and spaces, returns no token.
        // Return value: wheather program meets EOF.
        virtual bool eatCommentAndSpaces();
        // Scan punctuators
        virtual TokenPtr scanPunctuators();
        virtual TokenPtr scanKeywordOrIdentifier(string& word);
        virtual TokenPtr scanIdentifierAndStringCharacterLiteral();
        virtual TokenPtr scanStringCharacterLiteral(uint32_t encoding);
        virtual TokenPtr scanIntegerAndFloating();

    public:
        explicit Lexer();

        void openFile(const char * path) { M_sm->openFile(path); }
        void closeFile(const char * path) { M_sm->closeCurrFile(); }
        const string& getFileName() { M_sm->getCurrentFilename(); }

        void readch() { peak = M_sm->nextChar(); }
        void retract() { peak = M_sm->lastChar(); }
        string getLine(int i) { return M_sm->getLine(i); }
        string getLine() { return M_sm->getLine(); }
        string getCurrentLine() { return M_sm->getLine(); }
        void backToPos(int row, int col) { M_sm->to(row, col); }
        int getColumn() { return M_sm->getColumn(); }
        int getRow() { return M_sm->getRow(); }
        SourceManagerPtr getSourceManager() { return M_sm; }

        virtual TokenPtr scan();

        ~Lexer();
    };
}

#endif
