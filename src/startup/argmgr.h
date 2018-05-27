#ifndef _MIYUKI_ARGMGR_H
#define _MIYUKI_ARGMGR_H

#include <memory>
#include <string>
#include <iostream>

namespace Miyuki {

	class ArgumentManager;
	using ArgumentManagerPtr = std::shared_ptr<ArgumentManager>;

	class ArgumentManager {
	private:
		static ArgumentManagerPtr _instance;

		void printHelpMsg();
		void printVersionMsg();

		int argc;
		const char ** argv;

	public:
		// Flags
		std::string filename = "";
		std::string errMsg = "";
		std::string outputFileName = "a";

		bool flagEmitLLVMIR = false;
		bool flagEmitPreprocessed = false;
		bool flagCompileOnly = false;
		bool flagEnableQuickMath = false;
		bool flagLinkOnly = false;
		bool flagNoWarning = false;

		bool enableArgCountCheck = true;
		bool enableFileNameCountCheck = true;

		// Initialize argument manager
		bool init(int argc, const char ** argv);

	public:
		/// Static fields
		// Singleton Module
		static ArgumentManager& getInstance();
		
	};

}

#endif