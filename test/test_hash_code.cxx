#include "common/md5.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Miyuki::Common;

#define TEST_MD5(x) cout << setbase(16) << "  " << md5(x) << endl

int main() {
    while (1) {
        cout << "> ";
        string x;
        cin >> x;
        if (x == "quit") break;
        TEST_MD5(x.c_str());
    }
}