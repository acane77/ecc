#ifndef _MIYUKI_SRCMGR_H
#define _MIYUKI_SRCMGR_H

#include <stack>
#include <cassert>
#include <string>
#include <format.h>
#include "ptrdef.h"
#include "flread.h"
#include "observe.h"
#include "obsevent.h"

namespace Miyuki::Common {
    using namespace fmt;
    // This file defines source mamager
    //   Encapsulates the operation of the file switch

    DEFINE_SHARED_PTR(SourceManager)

    class SourceManager : public IObservable {
        // File read stack, stack top is the file we're reading
        std::deque<FileReadPtr> fileStack;
        // File which has been read, then add here for futher use
        std::deque<FileReadPtr> readFile;
        // Current file
        FileReadPtr  currFile;
        // flag that if gile reach end-of-file
        bool  reachEndOfFile = false;
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

        inline int _nextChar() {
            int ch = currFile->nextChar();
            if (ch == -1) {
                if ( reachEndOfFile )  {
                    notifyAll(ObserverEvent::SM_SWITCHING_FILE); // notify observer that switching event happened
                    reachEndOfFile = false;   //because we do not support retract from removed file, so just
                                               // set it to false after reach it
                }
                else reachEndOfFile = true;
                // If reach current file's end,
                // switch to last file
                if (getFileCount() > 1) {
                    closeCurrFile();
                    return '\n'; // if meet eof, then return new-line
                }
                // this is the only file , and reach end
                return -1;
            }
            // normally return character
            return ch;
        }
        int nextChar() {
            int ch = _nextChar();
            // eat \r
            if (ch == '\r') return nextChar();
            // eat \\\n
            if (ch == '\\') {
                ch = _nextChar();
                if (ch == '\n') return nextChar();

                lastChar(); return '\\';
            }
            return ch;
        }
        int lastChar() {
            //because we do not support retract from removed file, so just set it to false
            if (reachEndOfFile)  reachEndOfFile = false;
            return currFile->lastChar();
        }

        // Operation for file switching
        inline int getFileCount() { return fileStack.size(); }
        void openFile(const char * path) {
            if ( getFileCount() >= MaxStackSize )
                throw IOException("file stack too deeply");
            FileReadPtr fr = make_shared<FileRead>(path);
            if (currFile) {
                fr->includeFrom = currFile;
                fr->includeFromLine = getRow();
            }
            fileStack.push_back(fr);
            currFile = fr;
        }
        void closeCurrFile() {
            assert( currFile && "no file opened" );
            readFile.push_back(currFile);
            fileStack.pop_back();
            currFile = fileStack.back();
        }
        const string &getCurrentFilename() const { return currFile->getFilename(); }
        // get line from file stack
        string getLine(string filename, int line) {
            // first find files being read
            for (int i=0; i<fileStack.size(); ++i) {
                if (fileStack[i]->getFilename() == filename)
                    return fileStack[i]->getLine(line);
            }
            // then find read file
            for (int i=0; i<readFile.size(); ++i) {
                if (readFile[i]->getFilename() == filename)
                    return readFile[i]->getLine(line);
            }
            // not found
            return "{2}: Cannot find source file {0}:{1}"_format( filename, line, __FUNCTION__ );
        }
    };

}

#endif