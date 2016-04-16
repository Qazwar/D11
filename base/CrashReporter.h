#pragma once

namespace ds {

	class CrashReporter {

	public:
		virtual void saveReport() = 0;
		virtual void shutdown() = 0;
	};

	class CrashDump {

	public:
		CrashDump() {}
		void attach(CrashReporter* reporter);
		void dump();
		void exit();
	private:
		CrashReporter* _reporter;
	};

	extern CrashDump* gCrashDump;
}