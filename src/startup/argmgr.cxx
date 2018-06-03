#include "argmgr.h"

namespace Miyuki {

	ArgumentManagerPtr ArgumentManager::_instance = nullptr;

	bool ArgumentManager::init(int argc, const char ** argv) {
		ArgumentManager::argc = argc;
		ArgumentManager::argv = argv;

		if (enableArgCountCheck && argc <= 1) {
			printHelpMsg();
			return false;
		}
		for (int i = 1; i < argc; i++) {
			const char * arg = argv[i];
			if (arg[0] == '-') {
				std::string A(arg);
				if (A == "-emit-llvm") {
					flagEmitLLVMIR = true;
				}
				else if (A == "-emit-preprocessed") {
					flagEmitPreprocessed = true;
				}
				else if (A == "-c") {
					flagCompileOnly = true;
				}
				else if (A == "-enable-quickmath") {
					flagEnableQuickMath = true;
				}
				else if (A == "-l") {
					flagLinkOnly = true;
				}
				else if (A == "-no-warning") {
					flagNoWarning = true;
				}
				else if (A == "-v" || A == "-version") {
					printVersionMsg();
					return false;
				}
				else if (A == "-h" || A == "-help") {
					printHelpMsg();
					return false;
				}
				else if (A == "-o") {
					i++; arg = argv[i];
					bool failed = true;
					if (i < argc) {
						failed = false;
						if (arg[0] == '-')
							failed = true;
						else
							outputFileName = arg;
					}
					if (failed) {
						errMsg = errMsg + "ArgumentError:no output filename specified\n";
						return false;
					}
				}
				else {
					errMsg = errMsg + "ArgumentError: Unrecognized argument " + A + "\n";
					return false;
				}
			}
			else {
				if (enableFileNameCountCheck && filename != "") {
					errMsg = errMsg + "ArgumentError: More than one filename proviced.\n";
					return false;
				}
				filename = arg;
			}
		}
		if (filename == "") {
			errMsg = errMsg + "ArgumentError: No filename proviced.\n";
			return false;
		}
		return true;
	}

	ArgumentManager & ArgumentManager::getInstance() {
		if (_instance == nullptr) {
			_instance = std::make_shared<ArgumentManager>();
		}
		return *_instance;
	}

	void ArgumentManager::printHelpMsg() {
		std::cout << "usage: " << argv[0] << R"MiyukiRawString( <filename> [options]
    
Options:
    -emit-llvm            Generate LLVM IR instead
    -emit-preprocessed    Generate preprocessed soure file
    -c                    Compile only (do not link)
    -enable-quickmath     Enable QuickMath optimize.
    -l                    Link only.
    -no-warning           Do not generate warning
    -help     -h          Print this message
    -version  -v          Print version info
    -o <filename>         Specify output file name
)MiyukiRawString";
	}

	void ArgumentManager::printVersionMsg() {
		std::cout << R"MiyukiRawString(This is my graduate project, license under the MIT License.

Copyright 2018 Miyuki

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

)MiyukiRawString";
	}

}