#include <lex/pplex.h>
#include "lex/lex.h"
#include "lex/token.h"
#include "include.h"
#include "common/flread.h"
#include "common/exception.h"
#include "common/console.h"
#include "parse/parser.h"
#include "ast/env.h"
#include "argmgr.h"
#include "feature.h"

using namespace std;
using namespace Miyuki;
using namespace Miyuki::Parse;
using namespace Miyuki::Lex;

int main(int argc, const char ** argv) {
    cout << "ECC - Emilia C Compiler\n";
    cout << "Licensed under MIT License\n\n";

	ArgumentManager& AM = ArgumentManager::getInstance();
	AM.enableArgCountCheck = false;
	AM.enableFileNameCountCheck = false;
	AM.filename = "test.c";
	bool AMInitSuccess = AM.init(argc, argv);
	if (!AMInitSuccess) {
		cout << AM.errMsg;
		return 1;
	}

    const char * file_name = AM.filename.c_str();
	AST::TheModule->setSourceFileName(AM.filename);

    try {
		if (AM.flagEmitLLVMIR) {
			cout << "Emit LLVM IR. // TODO";
		}
		else if (AM.flagEmitPreprocessed) {
			startPreprocessor();
		}
		else if (AM.flagEnableQuickMath) {
			cout << "Enable quick math // TODO";
		}
		else if (AM.flagLinkOnly) {
			cout << "Enable link only // TODO";
		}
		else {
			startCompiling();
		}
    }
    catch (exception& e) {
        cout << Miyuki::Console::Error("fatal:") << e.what() << endl << endl;
		return -1;
    }

}
