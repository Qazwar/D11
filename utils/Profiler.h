#pragma once
#define PROFILING TRUE
#include <Vector.h>
#include <windows.h>
#include <chrono>
#include "Log.h"

namespace ds {

	class ReportWriter;

}

class StopWatch {

public:	
	StopWatch();
	explicit StopWatch(const char* name);
	~StopWatch();
	void start();
	void end();
	double elapsed();
	
private:
	std::chrono::steady_clock::time_point _start, _end;
	char _name[32];
	bool _running;
};

#define TIMER(name) StopWatch s(name); s.start();

// -------------------------------------------------------
// Profile data
// -------------------------------------------------------
struct ProfileSnapshot {

	char name[32];
	int level;
	int invokeCounter;
	float totalTime;

};

namespace perf {

	void init();

	void reset();

	void finalize();

	int start(const char* name);

	void end(int index);

	void shutdown();

	void debug();

	void save(const ds::ReportWriter& writer);

	//void showDialog(v2* position);

	//int get_snapshot(ProfileSnapshot* items, int max);

	float get_current_total_time();
}

class ZoneTracker {

public:
	ZoneTracker(const char* name) {
		_index = perf::start(name);
	}
	~ZoneTracker() {
		perf::end(_index);
	}
private:
	int _index;
};
