#ifndef _MIYUKI_FLREAD_H
#define _MIYUKI_FLREAD_H

#include <fstream>
#include <string>
#include <deque>
#include "common/exception.h"

using namespace std;

namespace Miyuki::Common {

    class FileRead {
        int column = 0;
        int row = 0;
        int max_row = 0;

        deque<string> lines;
        string line;

        fstream M_File;
    public:
        bool nextLine();
        bool lastLine();

    public:
        FileRead(const char * path);

        int getColumn() { return column; }
        int getRow() { return row; }
        string getLine(int i) { return lines[i]; }
        string getLine() { return line; }
        int to(int r, int c = 0) { row = r; column = c; line = lines[r]; }

        int nextChar();
        int lastChar();

        ~FileRead() { M_File.close(); }
    };

}

#endif
