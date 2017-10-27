#ifndef _MIYUKI_SRCMGR_H
#define _MIYUKI_SRCMGR_H

#include <stack>
#include <cassert>
#include "ptrdef.h"
#include "flread.h"

namespace Miyuki::Common {

    // This file defines source mamager
    //   Encapsulates the operation of the file switch

    DEFINE_SHARED_PTR(SourceManager)

    class SourceManager {
        std::stack<FileReadPtr> fileStack;
        FileReadPtr  currFile;
        enum {
            MaxStackSize = 300
        };
    public:
        SourceManager() { currFile = nullptr; }

        // Operations for the file
        bool nextLine() { return currFile->nextLine(); }
        bool lastLine() { return currFile->lastLine(); }

        int getColumn() { return currFile->getColumn(); }
        int getRow() { return currFile->getRow(); }
        string getLine(int i) { return currFile->getLine(i); }
        string getLine() { return currFile->getLine(); }
        int to(int r, int c = 0) { return currFile->to(r, c); }

        int nextChar() { return currFile->nextChar(); }
        int lastChar() { return currFile->lastChar(); }

        // Operation for file switching
        inline int getFileCount() { return fileStack.size(); }
        void openFile(const char * path) {
            if ( getFileCount() >= MaxStackSize )
                throw IOException("file stack too deeply");
            FileReadPtr fr = make_shared<FileRead>(path);
            fileStack.push(fr);
            currFile = fr;
        }
        void closeCurrFile() {
            assert( currFile && "no file opened" );
            currFile.reset();
            fileStack.pop();
        }
        const string &getCurrentFilename() const { return currFile->getFilename(); }
    };

}

#endif