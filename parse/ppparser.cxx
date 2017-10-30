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
        while (getCache()) {
            cout << "Cache Got **************\n";
            for (int i=0; i<cachedLine->size(); i++) {
                cout << "Test:  " << (*cachedLine)[i]->toString() << endl;
            }
            cout << endl;
        }
    }

    int FunctionLikeMacro::replace(TokenSequence &toksResult) {
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

    TokenSequencePtr PreprocessorParser::eval(TokenSequencePtr original) {
        TokenSequencePtr seqNew = make_shared<TokenSequence>();

        // first, we relace macros
        int replaceCount;
        do {
            replaceCount = 0;
            for (int i=0; i<original->size(); i++) {
                TokenPtr tok = (*original)[i];
                if (tok->isNot(Tag::Identifier)) {
                    // directly add to seqNew
                    seqNew->push_back(tok);
                    continue;
                }
                // else is identifier
                //   forst we should get to kow if this identifier is which one of the follow
                //   all possible types are:  macro,  function-like macro,  norml identifier(will be nnot reolaced)
                WordTokenPtr tokW = dynamic_pointer_cast<WordToken>(tok);
                // first we check if it is a macro
                MacroDefinePtr macroDef = macros.getMacroDef(tokW->name);
                if (!macroDef) {
                    // is not a macro
                    // directly add to seqNew
                    seqNew->push_back(tok);
                    continue;
                }
                // if is a function-like macro
                if (macroDef->isFunctionLike) {
                    // first we check if next token is '('
                    if (i != original->size() - 1   // has next item
                        && (*original)[i + 1]->is('(')) { // is a '('
                        // fetch all tokens
                        FunctionLikeMacroPtr macro = make_shared<FunctionLikeMacro>(macroDef);
                        int leftBracketCount = 1;
                        TokenSequencePtr param = make_shared<TokenSequence>();  // store first param (if there is)
                        while (leftBracketCount) {
                            // when ( is more than )
                            // get next token
                            tok = (*original)[++i];

                            if (tok->is(')')) {
                                // if meet a '(', left count -1
                                --leftBracketCount;
                            }
                            else if (tok->is(',')) {
                                // if meet a  comma, add last to parameter list
                                macro->params.push_back(param);
                                param = make_shared<TokenSequence>();
                                continue;
                            }
                            else if (i >= original->size()) {
                                // token runs out
                                // should not runs here
                                IParser::instance->diagError("unexpected new-line or eof", tok);
                                assert(false && "you should not run here");
                            }
                            else {
                                // add to param's token list
                                param->push_back(tok);
                            }
                        }
                        //add the last param (if exist)
                        if (param->size()) {
                            macro->params.push_back(param);
                        }
                        // repplace function macro
                        replaceCount += macro->replace(*seqNew);
                    }
                    // if this macro is used like a varible
                    else ; // do nothing
                }
                // is not a function-like macro
                else {
                    if (i != original->size() - 1   // has next item
                        && (*original)[i + 1]->is('(')) { // is a '('
                        IParser::instance->diagError("use a normal macro like a functon-like macro", tok);
                        // error recovery
                        // directly add this token to sequence
                        seqNew->push_back(tok);
                        continue;
                    }
                    // replace it
                    MacroPtr macro = make_shared<Macro>(macroDef);
                    replaceCount += macro->replace(*seqNew);
                }
            }
        }
        while (replaceCount);

        // TODO: then we calculate values in if or elif

        // return generated token sequence
        return seqNew;
    }

    bool PreprocessorParser::getCache() {
recache:
        cachedLine = make_shared<TokenSequence>();
        // check first token of the line
        next();
        // tell scanner if skip scanned line
        bool skipThisLine = false;
        // if first token is a group-part
        if (look->is('#')) {
            // get op-name
            skipThisLine =!setGroupPart( next() );
            // if is '#include' set flag to tell lexer return header name token
            if (dynamic_pointer_cast<WordToken>(look) && static_pointer_cast<WordToken>(look)->name == "include")
                M_pplex->setLexingContent(PreprocessorLexer::LexingContent::Include);

            for ( next() ; ; next() ) {
                if ( look->is('\n') || look->is(EOF) ) break;
                cachedLine->push_back(look);
            }
            // if is empty line, or this line is invalid, need recache
            int needRecache =  cachedLine->size() == 0 || skipThisLine;
            // if reach EOF
            if ( look->is(EOF) )  {
                // maybe last line, but this line is invalid
                if ( needRecache ) cachedLine = nullptr;
                return false;
            }
            if ( needRecache ) goto recache;
            return true;
        }

        else if (look->is(EOF)) {
            cachedLine = nullptr;
            return false;
        }

        // else is plain text-line
        int leftBracketCount = 0;
        bool isInFunction = true;
        for ( ; ; next() ) {
            if (look->is('\n')) {
                // for Multi-line function call, ignore this new-line
                if (isInFunction && leftBracketCount > 0) continue;
                // FIXME: add an addition 0
                break;
            }
                // readch end-of-file, Unconditional stop caching
            else if (look->is(EOF)) break;  // FIXME: add an addition 0
            cachedLine->push_back(look);
            if (look->is(Tag::Identifier)) {
                // is in function
                if (next()->is('(')) {
                    isInFunction = true;
                    leftBracketCount++;
                    cachedLine->push_back(look);
                    continue;
                }
                retract();
                continue;
            }
            else if (look->is(')')) {
                leftBracketCount--;
                continue;
            }
        }

        // if no token in sequence
        if ( cachedLine->size() == 0 ) {
            // meet end-of-file
            if ( look->is(EOF) )  {
                cachedLine = nullptr;
                return false;
            }
            // read next logical line
            goto recache;
        }

        return true;
    }

    bool PreprocessorParser::setGroupPart(TokenPtr op) {
        // note: if error occurred in this function,
        //  after receivng false return value, just skip the whole logical line,
        // and do not do any other processing
        int kind = -1;
        if (op->isNot(Tag::Identifier)) {
            groupPart = nullptr;
            if (op->is('\n') || op->is(EOF)) {
                // FOR control-line: # new-line
                return true;  // this struct is allowed in standard, so return true
            }
            // invalid
            return false;
        }
        WordTokenPtr opTok = static_pointer_cast<WordToken>(op);
        // Include = 0, If, Ifndef, Ifdef, Elif, Else, Endif, Define, Undef, Line, Error, Pragma, Empty, TextLine
        if (opTok->name == "include") kind = GroupPart::Include;
        else if (opTok->name == "if") kind = GroupPart::If;
        else if (opTok->name == "ifndef") kind = GroupPart::Ifndef;
        else if (opTok->name == "ifdef") kind = GroupPart::Ifdef;
        else if (opTok->name == "elif") kind = GroupPart::Elif;
        else if (opTok->name == "else") kind = GroupPart::Else;
        else if (opTok->name == "endif") kind = GroupPart::Endif;
        else if (opTok->name == "define") kind = GroupPart::Define;
        else if (opTok->name == "undef") kind = GroupPart::Undef;
        else if (opTok->name == "line") kind = GroupPart::Line;
        else if (opTok->name == "error") kind = GroupPart::Error;
        else if (opTok->name == "pragma") kind = GroupPart::Pragma;
        if (kind == -1) {
            groupPart = nullptr;
            return false;
        }
        groupPart = make_shared<GroupPart>(kind);
        return true;
    }
}


