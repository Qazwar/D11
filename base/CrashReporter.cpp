#include "CrashReporter.h" 

namespace ds {

	CrashDump* gCrashDump;

	void CrashDump::attach(CrashReporter* reporter) {
		_reporter = reporter;
	}

	void CrashDump::dump() {
		_reporter->saveReport();
		
	}

	void CrashDump::exit() {
		_reporter->shutdown();
	}

}