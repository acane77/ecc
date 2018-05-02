#include "ppparser.h"
#include "common/console.h"
#include "ast/ppastbuilder.h"

namespace Miyuki::Parse {
    MacroPack* MacroPack::instance;

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
        Lex::Token::flread = M_pplex->getSourceManager();
        while (1) {
            evalCachedLine();
            if (!evaledToks || evaledToks->size() == 0)  break;
            cout << Console::Yellow("Cache Got **************") << endl;
            cout << *evaledToks;
            cout << endl;
        }
        finish();
        parseDone();
        return;
        /*
         while (getCache()) {
            cachedLine = (cachedLine);
            cout << Console::Yellow("Cache Got **************") << endl;
            for (int i=0; i<cachedLine->size(); i++) {
                cout << "Test:  " << (*cachedLine)[i]->toString() << endl;
            }
            cout << endl;
        }
        return;*/
    }

    int FunctionLikeMacro::replace(TokenSequence &toksResult) {
        int replacementCount = 0;

        // process defined() if it is. if defination is nullptr, it must be defined
        if (macroName->toSourceLiteral() == "defined") {
            if (params.size() != 1 || params[0]->size() != 1 || (*params[0])[0]->isNot(Tag::Identifier)) {
                IParser::instance->diagError("operator \"defined\" requires an identifier", macroName);
                return replacementCount;
            }

            bool defined = MacroPack::getInstance()->getMacroDef( static_pointer_cast<WordToken>( (*params[0])[0])->name ) != nullptr;
            toksResult.push_back(make_shared<IntToken>(defined ? 1 : 0));
            replacementCount++;
            return replacementCount;
        }

        if (((!defination->isParamVarible) && defination->lparlen.size() != params.size()) || (defination->isParamVarible && defination->lparlen.size() > params.size())) {
            IParser::instance->diagError("'{0}' requires {1}{2} parameters, and {3} provided."_format( macroName->toSourceLiteral(), defination->isParamVarible ? "at least ":"", defination->lparlen.size() , params.size() ), macroName);
            return 0;
        }
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
                for (int k=0; k<tokSeq->size(); k++) {
                    strRet += (*tokSeq)[k]->toSourceLiteral();
                    if (k != tokSeq->size() - 1) strRet += " ";
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
                for (int i=0; i<tokSeq->size(); i++)
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

        if (name == "__VA_ARGS__") {
            // defination->lparlen.size() = named size
            // params.size() = call size
            for (int i=defination->lparlen.size(); i<params.size(); i++) {
                for (TokenPtr ptr : *(params[i])) {
                    tokSeq->push_back(ptr);
                }
                tokSeq->push_back(make_shared<Token>(','));
            }
            if (tokSeq->size())
                tokSeq->pop_back(); //remove last comma token
            return tokSeq;
        }
        int index = defination->getParamIndex(name);
        if (index == -1)   return nullptr;
        return params[index];
    }

    TokenSequencePtr PreprocessorParser::eval(TokenSequencePtr original) {
        TokenSequencePtr seqNew;

        // first, we relace macros
        int replaceCount;
        do {
            replaceCount = 0;
            seqNew = make_shared<TokenSequence>();
            for (int i=0; i<original->size(); i++) {
                TokenPtr tok = (*original)[i];
                if (tok->isNot(Tag::Identifier)) {
                    // if is two or more string connected
                    if (seqNew->size() && tok->is(Tag::StringLiteral) && seqNew->back()->is(Tag::StringLiteral)) {
                        // connect two strings
                        static_pointer_cast<StringToken>(seqNew->back())->value += static_pointer_cast<StringToken>(tok)->value;
                        continue;
                    }
                    // directly add to seqNew
                    seqNew->push_back(tok);
                    continue;
                }
                // else is identifier
                //   forst we should get to kow if this identifier is which one of the follow
                //   all possible types are:  macro,  function-like macro,  norml identifier(will be nnot reolaced)
                WordTokenPtr tokW = dynamic_pointer_cast<WordToken>(tok);
                // zero, we check if it is a predefined macro
                // replace line number (int)
                if (tokW->name == "__LINE__")   { seqNew->push_back(make_shared<IntToken>( Token::flread->getRow(), false ));  replaceCount++; goto replace_done; }
                    // replace file name (string-literal)
                else if (tokW->name == "__FILE__")  { seqNew->push_back(make_shared<StringToken>( Token::flread->getCurrentFilename().c_str(), Encoding::ASCII ));   replaceCount++; goto replace_done; }
                    // replace function name (string literal) //FIXME: after implement parser, write here
                else if (tokW->name == "__FUNC__")  { seqNew->push_back(make_shared<StringToken>( "to be implemented.", Encoding::ASCII ));  replaceCount++; goto replace_done; }

                // first we check if it is a macro
                MacroDefinePtr macroDef = macros.getMacroDef(tokW->name);
                if (!macroDef && tokW->name != "defined") {
                    // is not a macro
                    // directly add to seqNew
                    seqNew->push_back(tok);
                    continue;
                }
                // if is a function-like macro
                if (tokW->name == "defined" || macroDef->isFunctionLike) {
                    // first we check if next token is '('
                    if (i != original->size() - 1   // has next item
                        && (*original)[i + 1]->is('(')) { // is a '('
                        i++; // skip '('
                        // fetch all tokens
                        FunctionLikeMacroPtr macro = make_shared<FunctionLikeMacro>(macroDef);
                        macro->macroName = tokW;
                        int leftBracketCount = 1;
                        TokenSequencePtr param = make_shared<TokenSequence>();  // store first param (if there is)
                        // to resolve expression like 'F(F(1, 2), F(3, 4))'
                        bool prevTokenMaybeAFunctionName = false;
                        bool isInOtherFunctionParamList = false;
                        int  otherFunctionRemainingBracket = 0;

                        while (leftBracketCount) {
                            // when ( is more than )
                            // get next token
                            //cout << endl << i << ".";

                            if (i+1 >= original->size()) {
                                // token runs out
                                // should not runs here
                                diagError("bracket does not match. \n  note:  (maybe you can add {0} ')' at the end to solve this problem)"_format( leftBracketCount + otherFunctionRemainingBracket ), tok);
                                return seqNew;
                            }

                            tok = (*original)[++i];

                            //cout << tok->toSourceLiteral() << "  prevToken=" << prevTokenMaybeAFunctionName << "  isInOtherFunc=" << isInOtherFunctionParamList << "  otherFunctionRem=" << otherFunctionRemainingBracket ;
                            if (prevTokenMaybeAFunctionName) {
                                prevTokenMaybeAFunctionName = false;
                                if (tok->is('(')) {
                                    isInOtherFunctionParamList = true;
                                }
                            }

                            if (tok->is(')')) {
                                // if meet a ')', left count -1
                                if (isInOtherFunctionParamList)  {
                                    if (!--otherFunctionRemainingBracket)
                                        isInOtherFunctionParamList = false;
                                    goto add_tok;
                                }
                                else --leftBracketCount;
                            }
                            else if (tok->is('(')) {
                                // if meet a '(', left count +1
                                if (isInOtherFunctionParamList)  { ++otherFunctionRemainingBracket; goto add_tok; }
                                else ++leftBracketCount;
                            }
                            else if (tok->is(',')) {
                                // belongs to other function, add
                                if (isInOtherFunctionParamList)
                                    goto add_tok;
                                // if meet a  comma, add last to parameter list
                                macro->params.push_back(param);
                                param = make_shared<TokenSequence>();
                                continue;
                            }
                            else {
                                if (!isInOtherFunctionParamList && tok->is(Tag::Identifier)) {
                                    prevTokenMaybeAFunctionName = true;
                                }
                                add_tok:
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
                    // if this is a defined operator (non-bracket format)
                    else if (tokW->name == "defined"
                        &&  i != original->size() - 1   // has next item
                        && (*original)[i + 1]->is(Tag::Identifier) ) {
                        if (static_pointer_cast<WordToken>((*original)[i + 1])->name == "defined" /* exclude defined operator */) {
                            diagError("'defined' operator cannot use as an identifier.", (*original)[++i]);
                            continue; // replace other
                        }
                        FunctionLikeMacroPtr macro = make_shared<FunctionLikeMacro>(macroDef);
                        macro->macroName = tokW;
                        TokenSequencePtr param = make_shared<TokenSequence>();
                        param->push_back((*original)[++i]);
                        macro->params.push_back(param);
                        // replace define operator
                        replaceCount += macro->replace(*seqNew);
                    }
                    // if this macro is used like a varible
                    else ; // do nothing
                }
                // is not a function-like macro
                else {
                    // replace it
                    MacroPtr macro = make_shared<Macro>(macroDef);
                    macro->macroName = tokW;
                    replaceCount += macro->replace(*seqNew);
                }
            }
replace_done:
            original = seqNew;
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
            int needRecache = skipThisLine;
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
            groupPart = nullptr;
            return false;
        }

        // else is plain text-line
        groupPart = make_shared<GroupPart>(GroupPart::TextLine);
        int leftBracketCount = 0;
        bool isInFunction = true;
        // tell if a new-line has just ignored, be used to parsing 'new-line #'
        bool aNewLineJustIgnored = false;
        for ( ; ; next() ) {
            if (aNewLineJustIgnored) {
                aNewLineJustIgnored = false;
                if (look->is('#')) {
                    // is a preprocessing directive
                    retract(); // read more '#' so retract
                    break;
                }
            }
            if (look->is('\n')) {
                // for Multi-line function call, ignore this new-line
                if (isInFunction && leftBracketCount > 0) {
                    aNewLineJustIgnored = true;
                    continue;
                }
                // FIXME: add an addition 0
                break;
            }
                // readch end-of-file, Unconditional stop caching
            else if (look->is(EOF)) break;  // FIXME: add an addition 0
            cachedLine->push_back(look);
            if (look->is(Tag::Identifier)) {
                // we should not only read one more one token
                // if we found the next token is bew-lien new-line we should continue read
                // and get real meanningful token we want
                TokenPtr tok = next(), tokN = tok;
                if (tok->is('\n')) {
                    while (tok->is('\n')) {
                        // skip all new-line tokens
                        // get these token with no-caching
                        tok = M_lex->scan();
                    }
                    // find first non-new-line token
                    if (tok->is('('))  ;  // if itis s left-bracket, do nothing, fall
                    else {
                        // put this token to cache, and retract
                        //    for we read 2 more tokens ( new-line and the 'tok' ) then needed
                        cacheToken(tokN); cacheToken(tok);  retract(); retract();
                        continue;
                    }
                }
                // is in function
                if (tok->is('(')) {
                    isInFunction = true;
                    leftBracketCount++;
                    cachedLine->push_back(tok);
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
        groupPart->directiveTok = op;
        return true;
    }

    void PreprocessorParser::evalCachedLine() {
        do {
            cachedLine = nullptr;
            evaledToks = nullptr;
            getCache();
            if (!cachedLine)
                break;

            // is an empty sentense or invalid, who knows?
            if (!groupPart)
                continue;

            int kind = groupPart->kind;

            // consider change here condition judgement to array access to optmise
            if (kind == GroupPart::Include) {
                if (!getCondition())  continue;
                processInclude();
            }
            else if (kind == GroupPart::Define) {
                if (!getCondition())  continue;
                processDefine();
            }
            else if (kind == GroupPart::Undef) {
                if (!getCondition())  continue;
                processUndef();
            }
            else if (kind == GroupPart::TextLine) {
                if (!getCondition())  continue;
                processTextline();
            }
            else if (kind == GroupPart::Error) {
                if (!getCondition())  continue;
                processError();
            }
            else if (kind == GroupPart::Ifdef) {
                processIfdef();
            }
            else if (kind == GroupPart::Ifndef) {
                processIfndef();
            }
            else if (kind == GroupPart::Else) {
                processElse();
            }
            else if (kind == GroupPart::Endif) {
                processEndif();
            }
            else if (kind == GroupPart::If) {
                processIf();
                evaledToks = nullptr;  // for if and endif, evaledTok is only for evaluating expression (provide
                                       // tokens to lexer, so set it to zero to ensure it will not print to source
            }
            else if (kind == GroupPart::Elif) {
                processElif();
                evaledToks = nullptr;
            }
            else if (kind == GroupPart::Pragma) {
                processPragma();
            }
            else if (kind == GroupPart::Line) {
                processLine();
            }

            if (evaledToks && evaledToks->size()) evaledToksIter = evaledToks->begin();
        }
        while (!evaledToks || !evaledToks->size());
    }

#define nextTok() ( tok = (*cachedLine)[++i] )
#define currTok() ( tok = (*cachedLine)[i] )
#define noMoreTok() ( i+1 >= cachedLine->size() )
#define hasMoreToken() ( i+1 < cachedLine->size() )
#define getTok(i) ( (*cachedLine)[i] )
#define errorNoMoreToken() if (noMoreTok()) { diagError("unexpected new-line", tok); return; }
    void PreprocessorParser::processInclude() {
        cachedLine = eval(cachedLine);
        // check format
        //   headerName or stringLiteral
        if (cachedLine->size() == 1 && ((*cachedLine)[0]->is(Tag::StringLiteral) || (*cachedLine)[0]->is(Tag::HeaderName))) {
            string name = dynamic_pointer_cast<HeaderToken>((*cachedLine)[0]) ? dynamic_pointer_cast<HeaderToken>((*cachedLine)[0])->name :
                          ( dynamic_pointer_cast<StringToken>((*cachedLine)[0]) ? dynamic_pointer_cast<StringToken>((*cachedLine)[0])->value : "<file not avaible>" );
            // here set to defaultContent is import
            // I set this default value when I meet a new line,
            // but If an #include was not end with \n, lexingContent will not be set to default
            // and the lexer will lex defauleContent as Preprocess line, even a include groupPart
            // so this line is very important
            //// CELEBRATE !! FOUND THE BUG !!!!!
            M_pplex->setLexingContent(PreprocessorLexer::LexingContent::DefaultContent);

            try {
                M_lex->openFile(name.c_str());
                // TODO: undate line number
            }
            catch  (IOException& e) {
                diagError(e.what(), (*cachedLine)[0]); //filename token
            }
        }
        else diagError("file name expected.", groupPart->directiveTok);
    }

    void PreprocessorParser::processPragma() {
        // My implemetation does not implement #pragma
        return;
    }

    void PreprocessorParser::processLine() {
        // My implemetation does not implement #line
        return;
    }

    void PreprocessorParser::processDefine() {
        TokenPtr tok;
        // CHECK FORMAT
        //   #define name [ ( param , ...
        if (cachedLine->size() < 1) {
            diagError("no macro name given in #define directive", groupPart->directiveTok);
            return;
        }
        if ((*cachedLine)[0]->isNot(Tag::Identifier)) {
            diagError("macro names must be identifiers", groupPart->directiveTok);
            return;
        }
        if (static_pointer_cast<WordToken>((*cachedLine)[0])->name == "defined") {
            diagError("'define' cannot use as a macro name", (*cachedLine)[0]);
            return;
        }
        bool isAFunctionLikeMacro = cachedLine->size() >= 2 && (*cachedLine)[1]->is('(');
        MacroDefinePtr macroDef = make_shared<MacroDefine>();
        macroDef->isFunctionLike = isAFunctionLikeMacro;
        macroDef->isParamVarible = false;
        bool paramListClosed = false;
        if (isAFunctionLikeMacro) {
            int i = 2;
            // if no parameter
            currTok();
            if (tok->is(')'))
                goto parse_replacement_list;

            // parse paramter list
            for ( ; i < cachedLine->size(); i++ ) {
                currTok();
                if (tok->is(Tag::Ellipsis)) {
                    macroDef->isParamVarible = true;
                    errorNoMoreToken();
                    if (nextTok()->isNot(')')) {
                        diagError("missing ')' after {0} token"_format( tok->toSourceLiteral() ), tok);
                        return;
                    }
                    paramListClosed = true;
                    break;
                }
                if (tok->isNot(Tag::Identifier)) {
                    diagError("{0} may not appear in macro parameter list"_format( tok->toSourceLiteral() ), tok);
                    return; // give up this line, similarly hereinafter
                }
                // add to parameter list
                macroDef->lparlen.push_back(dynamic_pointer_cast<WordToken>(tok));
                errorNoMoreToken();
                if (nextTok()->is(')')) {
                    paramListClosed = true;
                    break;
                }
                if (tok->isNot(',') ) {
                    diagError("macro parameters must be comma-separated", tok);
                    return;
                }
            }
            // if param list is not closed
            if (!paramListClosed) {
                tok = getTok(i-1);
                diagError("')' required after {0} token"_format(tok->toSourceLiteral()), tok);
                return;
            }
parse_replacement_list:
            // parse replacement-list
            for ( i++ ; i < cachedLine->size(); i++ ) {
                currTok();
                macroDef->replacement.push_back(tok);
            }
        }
        else { // isAFunctionLikeMacro
            int i = 1;
            for ( ; i < cachedLine->size(); i++ ) {
                currTok();
                macroDef->replacement.push_back(tok);
            }
        }
        // add to macro pack
        if (!macros.addMacro(static_pointer_cast<WordToken>(getTok(0))->name, macroDef)) {
            diagWarning("'{0}' redefined"_format(getTok(0)->toSourceLiteral()), getTok(0));
            return;
        }
    }

    void PreprocessorParser::processUndef() {
        TokenPtr tok;
        // CHECK FORMAT
        //   #undef name
        if (cachedLine->size() < 1) {
            diagError(" no macro name given in #undef directive", groupPart->directiveTok);
            return;
        }
        tok = getTok(0);
        if (tok->isNot(Tag::Identifier)) {
            diagError("macro names must be identifiers", tok);
            return;
        }
        macros.removeMacroDef(tok->toSourceLiteral()); // same as cast to WordTokenPtr and access name
    }

    void PreprocessorParser::processTextline() {
        evaledToks = eval(cachedLine);
        convertToken();
    }

    void PreprocessorParser::processError() {
        string errmsg;
        for (TokenPtr ptr : *cachedLine)
            errmsg += ptr->toSourceLiteral() + " ";
        diagError(std::move(errmsg), groupPart->directiveTok);
    }

    void PreprocessorParser::processIfdef() {
        // check format
        if (cachedLine->size() < 1 ) {
            diagError("identifier name expected", groupPart->directiveTok);
            return;
        }
        if ((*cachedLine)[0]->isNot(Tag::Identifier)) {
            diagError("identifier name expected", (*cachedLine)[0] );
            return;
        }
        // check if macro is defined
        bool hasDefined = getCondition() && macros.getMacroDef( static_pointer_cast<WordToken>((*cachedLine)[0])->name ) != nullptr;
        addNewCondition(hasDefined);
    }

    void PreprocessorParser::processIfndef() {
    // check format
        if (cachedLine->size() < 1 ) {
            diagError("identifier name expected", groupPart->directiveTok);
            return;
        }
        if ((*cachedLine)[0]->isNot(Tag::Identifier)) {
            diagError("identifier name expected", (*cachedLine)[0] );
            return;
        }
        // check if macro is defined
        bool hasDefined = getCondition() && macros.getMacroDef( static_pointer_cast<WordToken>((*cachedLine)[0])->name ) == nullptr;
        addNewCondition(hasDefined);
    }

    void PreprocessorParser::processElse() {
        if (cachedLine->size() != 0) {
            diagWarning("extra tokens at end of #{0} directive"_format(groupPart->directiveTok->toSourceLiteral()), groupPart->directiveTok);
        }
        if (condHierarchy.size() == 0) {
            diagError("#else without #if", groupPart->directiveTok);
            return;
        }
        // negitive condition value
        // if parent is true,
        if (condHierarchy.back()->parentIsTrue) {
            negateCondition();
        }
    }

    void PreprocessorParser::processElif() {
        using namespace Miyuki::AST;

        if (cachedLine->size() == 0) {
            diagError("conditional expression expected.", groupPart->directiveTok);
            return;
        }
        if (condHierarchy.size() == 0) {
            diagError("#elif without #if", groupPart->directiveTok);
            return;
        }
        if (getCondition()) {
            // if condition is true, negative the condition
            if (condHierarchy.back()->parentIsTrue) {
                negateCondition();
            }
            return;
        }
        // if condition is false and used to be true, it also be false
        //   for this situation: #if 0 ... [#elif 1] ... [#elif 0] ... [#elif 1 <this>] ... #endif
        else if (condHierarchy.back()->_conditionUsedToBeTrue) {
            return;
        }

        evaledToks = eval(cachedLine);
        convertToken();

        if (!evaledToks)  return;
        PreprocessorASTBuilder ast(evaledToks);
        ExpressionPtr astRoot = ast.constantExpression();
        if (!ast.success() || !astRoot) {
            diagError("conditional expression is invalid.", groupPart->directiveTok);
            addNewCondition(0);
            return;
        }
        // set flag to allow && and || eval
        Symbol::isPreprocessorSymbol = true;
        astRoot->eval();
        Symbol::isPreprocessorSymbol = false;
        cout << Console::Green("processIf()  ");
        if (astRoot->IsCalculated())
            cout << "expression is calculated, value is " << astRoot->getCalculatedToken()->toInt() << endl;
        else {
            cout << "expression is not calculated\n";
            diagError("conditional expression is not evalulatable.", groupPart->directiveTok);
            addNewCondition(0);
            return;
        }

        // negitive condition value
        // if parent is true, and condition is true
        if (astRoot->getCalculatedToken()->toInt() != 0 && condHierarchy.back()->parentIsTrue) {
            negateCondition();
        }
    }

    void PreprocessorParser::processEndif() {
        if (cachedLine->size() != 0) {
            diagWarning("extra tokens at end of #{0} directive"_format(groupPart->directiveTok->toSourceLiteral()), groupPart->directiveTok);
        }
        if (condHierarchy.size() == 0) {
            diagError("#endif without #if", groupPart->directiveTok);
            return;
        }
        // pop up top
        endCurrentCondition();
    }

    void PreprocessorParser::processIf() {
        using namespace Miyuki::AST;

        if (cachedLine->size() == 0) {
            diagError("conditional expression expected.", groupPart->directiveTok);
            return;
        }

        evaledToks = eval(cachedLine);
        convertToken();

        if ( !evaledToks )  return;
        PreprocessorASTBuilder ast(evaledToks);
        ExpressionPtr astRoot = ast.constantExpression();
        if (!ast.success() || !astRoot) {
            diagError("conditional expression is invalid.", groupPart->directiveTok);
            addNewCondition(0);
            return;
        }
        // set flag to allow && and || eval
        Symbol::isPreprocessorSymbol = true;
        astRoot->eval();
        Symbol::isPreprocessorSymbol = false;
        cout << Console::Green("processIf()  ");
        if (astRoot->IsCalculated())
            cout << "expression is calculated, value is " << astRoot->getCalculatedToken()->toInt() << endl;
        else {
            cout << "expression is not calculated\n";
            diagError("conditional expression is not evalulatable.", groupPart->directiveTok);
            addNewCondition(0);
            return;
        }

        addNewCondition(astRoot->getCalculatedToken()->toInt() != 0);
    }

    bool PreprocessorParser::getCondition() {
        if (condHierarchy.size() == 0)  return true;
        return condHierarchy.back()->isTrue();
    }

    bool PreprocessorParser::setCondition(bool c) {
        if (condHierarchy.size() == 0)  return false;
        condHierarchy.back()->set(c);
    }

    bool PreprocessorParser::endCurrentCondition() {
        if (condHierarchy.size() == 0)  return false;
        condHierarchy.pop_back();
        return true;
    }

    bool PreprocessorParser::negateCondition() {
        if (condHierarchy.size() == 0)  return false;
        condHierarchy.back()->set( !condHierarchy.back()->isTrue() );
    }

    void PreprocessorParser::addNewCondition(bool c) {
        condHierarchy.push_back(make_shared<PreprocCondition>(c, groupPart->directiveTok, getCondition() ));
    }

    void PreprocessorParser::convertToken() {
        size_t seqSize = evaledToks->size();
        // cout << "**** Tokens before convert\n" << *evaledToks << endl;
        for (size_t i=0; i < seqSize; i++ ) {
            TokenPtr tok = (*evaledToks)[i];
            if ( tok->is(Tag::PPLiteral) || tok->is(Tag::PPNumber) || tok->is(Tag::Identifier) )
                (*evaledToks)[i] = M_imlex->getRealToken( static_pointer_cast<PPLiteralToken>(tok) );
        }
    }

    void PreprocessorParser::finish() {
        // check if all #if or #ifdef are closed
        if (condHierarchy.size()) {
            // travel all unterminated #if or #ifdef directive
            for (PreprocConditionPtr cond : condHierarchy) {
                diagError("unterminated #{0}"_format(cond->tok->toSourceLiteral()), cond->tok);
            }
        }
    }

    void PreprocessorParser::prepareGetToken() {
        Lex::Token::flread = M_pplex->getSourceManager();
        evalCachedLine();
        if (evaledToks && evaledToks->size())
            evaledToksIter = evaledToks->begin();
    }

    TokenPtr PreprocessorParser::nextToken() {
rescan:
        // if reach the end
        if (!evaledToks || evaledToks->size() == 0)
            return make_shared<Token>(Tag::EndOfFile);

        // if this line does not reach the end
        if (evaledToksIter != evaledToks->end()) {
            // return the token'
            return *(evaledToksIter++);
        }
        // else reach end
        evalCachedLine();
        if (evaledToks && evaledToks->size())
            evaledToksIter = evaledToks->begin();
        goto rescan;
        assert( false && "you shouldn't go here." );
    }
}


