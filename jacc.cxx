#include <lex/pplex.h>
#include "lex/lex.h"
#include "lex/token.h"
#include "include.h"
#include "common/flread.h"
#include "common/exception.h"

using namespace std;
int main() {
    cout << "JACC - Java Code C\n";
    cout << "License under MIT License\n\n";

    Miyuki::Common::FileReadPtr read = make_shared<Miyuki::Common::FileRead>( "test.c" );
    Miyuki::Lex::Lexer lexer(read);

    Miyuki::Lex::Token::flread = read;
    try {
        Miyuki::Lex::TokenPtr ptr = lexer.scan();
        while (ptr->tag != Miyuki::Lex::Tag::EndOfFile) {
            cout << ptr->toString() << endl;
            ptr = lexer.scan();
        }
    }
    catch (std::exception& e) {
        string s = read->getLine();
        cout << s << endl;
        int tabcount = 0;
        for (int i=0; i<s.length(); i++)
            if (s[i] == '\t') { cout << "\t"; tabcount++; }
        for (int i=1; i<Miyuki::Lex::Token::startColumn; i++)
            cout << " ";
        for (int i=Miyuki::Lex::Token::startColumn; i<read->getColumn(); i++)
            cout << "~";
        cout << endl << "" << read->getRow() << ":" << read->getColumn() << ": ";
        cout << e.what();
    }
}
