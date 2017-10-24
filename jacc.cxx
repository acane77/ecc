#include "include/include.h"
#include "common/flread.h"

using namespace std;
int main() {
    cout << "JACC - Java Code C\n";
    cout << "License under MIT License\n\n";

    Miyuki::Common::FileRead read("test.c");
    int c;
    while ((c = read.nextChar()) != -1) {
        //cout << (char)c;
    }
    while ((c = read.lastChar()) != -1) {
        cout << (char)c;
    }
}
