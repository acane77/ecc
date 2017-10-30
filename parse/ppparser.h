#ifndef _MIYUKI_PPPARSER_H
#define _MIYUKI_PPPARSER_H

#include "ifparser.h"
#include "lex/pplex.h"
#include <unordered_map>

namespace Miyuki::Parse {

    using namespace Lex;

    typedef deque<TokenPtr> TokenSequence;
    typedef shared_ptr<TokenSequence> TokenSequencePtr;

    DEFINE_SHARED_PTR(MacroDefine)
    DEFINE_SHARED_PTR(MacroPack)
    DEFINE_SHARED_PTR(Macro)
    DEFINE_SHARED_PTR(FunctionLikeMacro)
    DEFINE_SHARED_PTR(GroupPart)

    // macro defination.
    class MacroDefine {
    public:
        string macroName;
        bool   isFunctionLike;
        // for function-like macros
        bool   isParamVarible;
        deque<WordTokenPtr> lparlen;
        TokenSequence replacement;

        int getParamIndex(string& name) {
            for (int i=0; i<lparlen.size(); i++)
                if (lparlen[i]->name == name)
                    return i;
            return -1;
        }
    };

    // store a pack of macro
    class MacroPack {
        static MacroPack instance;
    public:
        unordered_map<string, MacroDefinePtr> macros;

        static MacroPack &getInstance() { return instance; }
        MacroDefinePtr getMacroDef(string name, bool isFunctionLike) { // FIXME: remove? maybe useless
            auto it = macros.find(name);
            if (it != macros.end() && it->second->isFunctionLike == isFunctionLike) return it->second;
            return nullptr;
        }
        MacroDefinePtr getMacroDef(string name) {
            auto it = macros.find(name);
            if (it != macros.end()) return it->second;
            return nullptr;
        }
    };

    // use macro (directly use or call)
    class Macro {
    public:
        MacroDefinePtr defination;
        bool isFunctionLike;

        explicit Macro(MacroDefinePtr def) {
            defination = def;
        }
        // Incremental add tokens to toksResult
        // return values: number of token replaced.
        virtual int replace(TokenSequence& toksResult) {
            // copy replacement to token sequence, same as replacement
            size_t replen = defination->replacement.size();
            for (int i=0; i<replen; i++) {
                toksResult.push_back(defination->replacement[i]);
            }
            return 1;
        }
    };

    // group-part (ref A.2.4)
    class GroupPart {
    public:
        uint32_t kind;

        enum : uint32_t {
            Include = 0, If, Ifndef, Ifdef, Elif, Else, Endif, Define, Undef, Line, Error, Pragma, Empty, TextLine
        };

        GroupPart(uint32_t _kind) : kind(_kind) {  }

        bool is(uint32_t _tag) { return kind == _tag; }
        void process();
    };

    class FunctionLikeMacro : public Macro {
    public:
        deque<TokenSequencePtr> params;

        FunctionLikeMacro(MacroDefinePtr def) : Macro(def) {

        }

        bool isParameter(string name) {
            for (WordTokenPtr tok : defination->lparlen) {
                if (tok->name == name)
                    return true;
            }
            return false;
        }

        // get parameter name from value
        //   value structure is unnamed, so we should get names structure and map them
        //   to find the value we want,
        //   In addition, we do special process on __VA_ARGS__ macro, if we know it is
        //   __VA_ARGS__, we marge all item in max-named-param size
        //   If there is a ... in the identifier-list in the macro definition, then the trailing arguments,
        //   including any separating comma preprocessing tokens, are merged to form a single item:
        //   the variable arguments. The number of arguments so combined is such that, following
        //   merger, the number of arguments is one more than the number of parameters in the macro
        //   definition (excluding the ...).
        TokenSequencePtr getParameterValue(string& name);

        // note: when add replacement list, add an additional 0 for convience
        int replace(TokenSequence& toksResult) override;
    };

    // This file defined preprocessor parser
    class PreprocessorParser : public IParser {
        PreprocessorLexerPtr M_pplex;

        // Save a line of Token, read token until new-line or EOF (*), and try replace on
        // this line of tokens, until no replacement being taken, write to file and start
        // next cache.
        // for command line: skip # and token with special meaning,
        // for text line: Recursively descend to read the structure, util encount a new-line
        // note2; structure for text-line has only one situation. function call.
        //    no matter it is or function or a function-like macro.
        TokenSequencePtr cachedLine;

        // macro defined.
        MacroPack macros;

        // group part type info
        GroupPartPtr groupPart;

    public:
        explicit PreprocessorParser(const char * path) : M_pplex(make_shared<PreprocessorLexer>()) {
            M_pplex->openFile(path);
            M_lex = M_pplex;
            registerObserver();
        }

        void testLexer();
        void parse() final;

        void parseControlLine();

        // get group-part kind from opTok
        // return value: true if is a valid group part,
        //               false if invalid
        bool setGroupPart(TokenPtr op);

        // cache a line with full syntax meaning from lexer
        // return value: true: there're lines did not read
        //               false: there's no line did not read
        // dide effect:  set cachedLine, if the value is nullptr,
        //               means the value is invalid, and no more line to read
        bool getCache();

        // calculate values
        TokenSequencePtr eval(TokenSequencePtr seq);
    };

}

#endif