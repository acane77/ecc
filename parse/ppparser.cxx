#include "ppparser.h"
#include "common/console.h"

namespace Miyuki::Parse {
    void PreprocessorParser::testLexer() {
        Lex::Token::flread = M_pplex->getSourceManager();
        next();
        //cout << (int)look->tag;
        while (look->isNot(-1)) {
            cout << Console::Cyan(__FUNCTION__) << look->toString() << endl;
            next();
        }
        parseDone();
    }

    void PreprocessorParser::parse() {

    }

    int FunctionLikeMacro::replace(TokenSequence &toksResult) {
        {
            int replacementCount = 0;
            size_t replen = defination->replacement.size();
            for (int i=0; i<replen; i++) {
                TokenPtr tok = defination->replacement[i];

                //  SHARP to string
                if (tok->is('#')) {
                    // get next token
                    WordTokenPtr tok = dynamic_pointer_cast<WordToken>( defination->replacement[++i] );
                    if (!(tok && isParameter( tok->name ))) {
                        IParser::instance->diagError("'#' is not followed by a macro parameter", tok);
                        continue;
                    }
                    // generate string from name
                    // this is macro parameter name
                    string paramName = tok->name;
                    // then through the paramName from the incoming parameters to find TokenSequence
                    // aka parameter value
                    TokenSequencePtr tokSeq = getParameterValue(paramName);
                    // if is # just convert to string directly, do not do any other convertion
                    string strRet;
                    for (TokenPtr& tok : *tokSeq) {
                        strRet += tok->toSourceLiteral() + " ";
                    }
                    toksResult.push_back(make_shared<StringToken>( strRet,  Encoding::ASCII ));
                    replacementCount++;
                    continue;
                }

                // DOUBLE SHARP paste
                //   directly paste x of ##x on previous token
                if (tok->is(Tag::DoubleSharp)) {
                    if (i == 0 || i == replen - 1) {
                        IParser::instance->diagError("'##' cannot at either start or end of the expansion", tok);
                        continue;
                    }
                    WordTokenPtr tok = dynamic_pointer_cast<WordToken>( defination->replacement[++i] );

                    // PASTE token on last token
                    // if last token is identifier
                    TokenPtr lastTok = toksResult.back();
                    WordTokenPtr wordPtr = make_shared<WordToken>("");
                    toksResult.pop_back();
                    if ( dynamic_pointer_cast<WordToken>( lastTok ) || dynamic_pointer_cast<PPNumberToken>( lastTok ) || dynamic_pointer_cast<PPLiteralToken>( lastTok ) ) {
                        wordPtr->name = lastTok->toSourceLiteral();
                    }
                    else {
                        IParser::instance->diagError("invalid use of '##' ", tok);
                        toksResult.push_back(lastTok);  // this token is invalid, put back
                        continue;
                    }
                    if (tok && isParameter( tok->name )) {
                        //get parameter value
                        TokenSequencePtr tokSeq = getParameterValue(tok->name);
                        //Is a complete Token, e.g. 123
                        // directly paste on last token
                        wordPtr->name += (*tokSeq)[0]->toSourceLiteral();
                        toksResult.push_back(wordPtr);
                        if (tokSeq->size() == 1) ;//do nothing
                            // else insert new token
                        else {
                            for (int i=1; i<tokSeq->size(); i++)
                                toksResult.push_back((*tokSeq)[i]);
                        }
                    }
                    else {//directly paste literal value
                        wordPtr->name += tok->name;
                        toksResult.push_back(wordPtr);
                    }
                    replacementCount++;
                    continue;
                }

                // if is parameter name
                WordTokenPtr tokW = dynamic_pointer_cast<WordToken>(tok);
                if ( tokW && isParameter( tokW->name ) ) {
                    TokenSequencePtr tokSeq = getParameterValue(tokW->name);
                    for (int i=1; i<tokSeq->size(); i++)
                        toksResult.push_back((*tokSeq)[i]);
                    replacementCount++;
                    continue;
                }

                // else directly write
                toksResult.push_back(tok);
            }
            return replacementCount;
        }
    }

    TokenSequencePtr FunctionLikeMacro::getParameterValue(string &name) {
        TokenSequencePtr tokSeq = make_shared<TokenSequence>();
        assert( name != "__VA_ARGS__" && "invalid use of getParameterValue" );

        // return line number (int)
        if (name == "__LINE__")   { tokSeq->push_back(make_shared<IntToken>( Token::flread->getRow(), false ));  return tokSeq; }
            // return file name (string-literal)
        else if (name == "__FILE__")  { tokSeq->push_back(make_shared<StringToken>( Token::flread->getCurrentFilename().c_str(), Encoding::ASCII ));  return tokSeq; }
            // return function name (string literal) //FIXME: after implement parser, write here
        else if (name == "__FUNC__")  { tokSeq->push_back(make_shared<StringToken>( "to be implemented.", Encoding::ASCII )); return tokSeq; }
            // return args list
        else if (name == "__VA_ARGS__") {
            // defination->lparlen.size() = named size
            // params.size() = call size
            for (int i=defination->lparlen.size(); i<params.size(); i++)
                for (TokenPtr ptr : *(params[i]))
                    tokSeq->push_back(ptr);
        }
        int index = defination->getParamIndex(name);
        if (index == -1)   return nullptr;
        return params[index];
    }
}


