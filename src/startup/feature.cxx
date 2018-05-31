#include "feature.h"
#include "parse/ppparser.cxx"
#include "parse/parser.h"
#include "ast/env.h"
#include "ast/astbuilder.h"

namespace Miyuki {

	const char * getInputFileName() {
		ArgumentManager& AM = ArgumentManager::getInstance();
		return AM.filename.c_str();
	}

	void startPreprocessor(std::ostream& output_os) {
		using namespace Miyuki::AST;

		Parser parser(getInputFileName());
		parser.getPreprocessedSource(output_os);
		
	}

	void startCompiling() {
		using namespace Miyuki::AST;
		using namespace Miyuki::Parse;

		Parser parser(getInputFileName());
		AST::TranslationUnitPtr AST;

		parser.parse();
		AST = parser.getAST();
		parser.parseDone();
		AST->gen();
		parser.parseDone();
		AST::TheModule->dump();
	}

}