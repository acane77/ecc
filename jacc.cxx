#include <lex/pplex.h>
#include "lex/lex.h"
#include "lex/token.h"
#include "include.h"
#include "common/flread.h"
#include "common/exception.h"

using namespace std;
int main(int argc, const char ** argv) {
    cout << "JACC - Java Code C\n";
    cout << "License under MIT License\n\n";

    Miyuki::Lex::PreprocessorLexer lexer;
    if (argc <= 1) lexer.openFile( "test.c" );
    for (int i=1; i<argc; i++)
        lexer.openFile(argv[i]);

    Miyuki::Lex::Token::flread = lexer.getSourceManager();

    try {
        Miyuki::Lex::TokenPtr ptr = lexer.scan();
        while (ptr->tag != Miyuki::Lex::Tag::EndOfFile) {
            cout << ptr->toString() << " \n";
            ptr = lexer.scan();
        }
    }
    catch (std::exception& e) {
        string s = lexer.getLine();
        cout << s << endl;
        int tabcount = 0;
        for (int i=0; i<s.length(); i++)
            if (s[i] == '\t') { cout << "\t"; tabcount++; }
        for (int i=1; i<Miyuki::Lex::Token::startColumn; i++)
            cout << " ";
        for (int i=Miyuki::Lex::Token::startColumn; i<lexer.getColumn(); i++)
            cout << "~";
        cout << endl << "" << lexer.getRow() << ":" << lexer.getColumn() << ": ";
        cout << e.what();
    }
}
