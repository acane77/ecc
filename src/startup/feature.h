#ifndef _MIYUKI_FEATURE_H
#define _MIYUKI_FEATURE_H

#include "argmgr.h"
#include "ast/env.h"
#include "parse/parser.h"

namespace Miyuki {
	
	// Get input file name
	const char * getInputFileName();

	// Start preprocessor
	void startPreprocessor(std::ostream& output_os = std::cout);

	// start Parsing
	void startCompiling();
}

#endif