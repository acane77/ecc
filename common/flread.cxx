#include "flread.h"

namespace Miyuki::Common {

    FileRead::FileRead(const char *path) {
        M_File.open(path, ios_base::in | ios_base::out);
        if (!M_File) throw IOException();
        filename = path;
    }

    int FileRead::nextChar() {
        while (column >= line.length()) {
            if (!nextLine()) return -1;
            column = 0;
        }
        return line[column++];
    }

    int FileRead::lastChar() {
        while (column-1 < 0) {
            if (!lastLine()) return -1;
            column = line.length();
        }
        return line[--column];
    }

    bool FileRead::nextLine() {
        char p;
        string line_save = line;
        line = "";

        if (max_row != row) {
            // Current row is not the last row
            if (row + 1 >= lines.size()) return false;
            column = 0;
            line = lines[++row];
            return true;
        }

        // Current row is last row
        bool enc_eof = false;
        while (1) {
            // Read and append char to string until new_line or EOF
            while (1) {
                M_File.get(p);
                if (M_File.eof()) { enc_eof = true; break; }
                if (p != '\r') line += p;  // ignore CR
                if (p == '\n') break;
            }

            if (line.length()) {
                column = 0;
                row++;
                max_row++;
                lines.push_back(line_save);
                return true;
            }
            if (enc_eof) {
                // For convince of retract, here still column add 1
                column++;
                break;
            }
        }

        // Restore saved line.
        line = line_save;
        return false;
    }

    bool FileRead::lastLine() {
        if (row == 0) return false;
        column = 0;
        line = lines[--row];
        return true;
    }
}
