#include "ppastbuilder.h"

namespace Miyuki::AST {

    TokenPtr PreprocessorASTBuilder::next() {
        if (m_tsptr_r == m_tsptr_w) {
            // token runs out
            if ( tokIndex >= src->size() )
                look = make_shared<Token>(Tag::EndOfFile);
            else
                look = (*src)[ tokIndex ];
            tokens[(++m_tsptr_w) % MaxRetractSize] = look;
            m_tsptr_r++;
            return look;
        }
        else if (m_tsptr_r < m_tsptr_w) {
            look = tokens[(++m_tsptr_r) % MaxRetractSize];
            return look;
        }
        else assert( false && "stack overflow." );
    }
}