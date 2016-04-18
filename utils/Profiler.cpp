#include "Profiler.h"
#include "Log.h"
#include "StringUtils.h"
//#include "..\ui\IMGUI.h"
//#include "..\DialogResources.h"
#include <algorithm>
#include <functional>

StopWatch::StopWatch() {
	//QueryPerformanceFrequency(&_frequency);
	_running = false;
	sprintf_s(_name, 32, "StopWatch");
}

StopWatch::StopWatch(const char* name) {
	//QueryPerformanceFrequency(&_frequency);
	_running = false;
	sprintf_s(_name, 32, name);
}

StopWatch::~StopWatch() {
	if (_running) {
		end();
		LOG << _name << " - elapsed: " << elapsed();
	}
}

void StopWatch::start() {
	_start = _end = std::chrono::steady_clock::now();
	_running = true;
}

//double StopWatch::LIToSecs(LARGE_INTEGER & L) {
	//return ((double)L.QuadPart * 1000.0 / (double)_frequency.QuadPart);
//}

void StopWatch::end() {
	_running = false;
	_end = std::chrono::steady_clock::now();
}

double StopWatch::elapsed() {
	auto duration = _end - _start;
	auto time_span = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

	//auto delta = std::chrono::duration_cast<double>(_end - _start);
	return static_cast<double>(time_span) / 1000.0;
}

namespace profiler {

	const int MAX_PROFLING_ENTRIES = 256;

	// -------------------------------------------------------
	// Profile event type
	// -------------------------------------------------------
	enum ProfileEventType {
		PET_START,
		PET_END,
		PET_EOL
	};

	// -------------------------------------------------------
	// Profile event
	// -------------------------------------------------------
	struct ProfileEvent {
		int parent;
		LARGE_INTEGER startingTime;
		float duration;
		ProfileEventType type;
		int name_index;
		IdString hash;
	};
	
	// -------------------------------------------------------
	// Profile data
	// -------------------------------------------------------
	struct ProfileData {

		const char* name;
		int hash;
		int level;
		int invokeCounter;
		int parent;
		float totalTime;
		float subTime;
		LARGE_INTEGER startingTime;
	};

	// -------------------------------------------------------
	// Profile context
	// -------------------------------------------------------
	struct ProfileContext {

		ProfileData data[MAX_PROFLING_ENTRIES];
		int index;
		int current;
		LARGE_INTEGER frequency;
		float totalTime;
		float totalTimes[32];
		int numTimes;
		int totalTimesIndex;
	};

	static ProfileContext ctx;

	// -------------------------------------------------------
	// initialize profiling system
	// -------------------------------------------------------
	void init() {
		ctx.numTimes = 0;
		ctx.totalTimesIndex = 0;
		for (int i = 0; i < 32; ++i) {
			ctx.totalTimes[i] = 0.0f;
		}
		ProfileData& pd = ctx.data[0];
		pd.name = "root";
		pd.level = 0;
		pd.parent = -1;
		pd.invokeCounter = 0;
		pd.totalTime = 0.0f;
		pd.subTime = 0.0f;
		QueryPerformanceFrequency(&ctx.frequency);
	}

	// -------------------------------------------------------
	// reset frame
	// -------------------------------------------------------
	void reset() {
		ctx.index = 1;
		ctx.current = 0;
		ctx.totalTime = 0;
		ctx.data[0].totalTime = 0;
		QueryPerformanceCounter(&ctx.data[0].startingTime);
	}

	// -------------------------------------------------------
	// build hash
	// -------------------------------------------------------
	int hash(const char *key) {
		int hash = 0;
		int len = strlen(key);
		for (int i = 0; i < len; ++i) {
			hash += key[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}

	// -------------------------------------------------------
	// find entry
	// -------------------------------------------------------
	int find(int hash) {
		for (int i = 0; i < ctx.index; ++i) {
			if (ctx.data[i].hash == hash) {
				return i;
			}
		}
		return -1;
	}

	// -------------------------------------------------------
	// start profiling sector
	// -------------------------------------------------------
	void start(const char* name) {
		int hashName = hash(name);
		int index = find(hashName);
		if (index == -1 && ctx.index < MAX_PROFLING_ENTRIES) {
			int idx = ctx.index++;
			ProfileData& pd = ctx.data[idx];
			pd.invokeCounter = 1;
			pd.totalTime = 0.0f;
			pd.subTime = 0.0f;
			pd.level = ctx.data[ctx.current].level + 1;
			pd.parent = ctx.current;
			pd.name = name;
			pd.hash = hashName;
			ctx.current = idx;
			QueryPerformanceCounter(&pd.startingTime);
		}
		else {
			ProfileData& pd = ctx.data[index];
			QueryPerformanceCounter(&pd.startingTime);
			++pd.invokeCounter;
		}
	}

	double LIToSecs(LARGE_INTEGER & L) {
		return ((double)L.QuadPart * 1000.0 / (double)ctx.frequency.QuadPart);
	}

	// -------------------------------------------------------
	// stop profiling sector
	// -------------------------------------------------------
	void end(const char* name) {
		int hashName = hash(name);
		int index = find(hashName);
		if (index != -1) {
			LARGE_INTEGER EndingTime;
			QueryPerformanceCounter(&EndingTime);
			LARGE_INTEGER time;
			time.QuadPart = EndingTime.QuadPart - ctx.data[index].startingTime.QuadPart;
			float elapsed = LIToSecs(time);
			ctx.totalTime += elapsed;
			ctx.data[index].totalTime += elapsed;
			//ctx.data[index].invokeCounter += 1;
			ctx.current = ctx.data[index].parent;
		}
	}

	// -------------------------------------------------------
	// finalize profiling frame
	// -------------------------------------------------------
	void finalize() {
		LARGE_INTEGER EndingTime;
		QueryPerformanceCounter(&EndingTime);
		LARGE_INTEGER time;
		time.QuadPart = EndingTime.QuadPart - ctx.data[0].startingTime.QuadPart;
		float elapsed = LIToSecs(time);
		ctx.totalTime = elapsed;
		ctx.data[0].totalTime = elapsed;
		ctx.totalTimes[ctx.totalTimesIndex++] = elapsed;
		if (ctx.totalTimesIndex >= 32) {
			ctx.totalTimesIndex = 0;
		}
		if (ctx.numTimes < 32) {
			++ctx.numTimes;
		}
	}

	float get_current_total_time() {
		return ctx.data[0].totalTime;
	}
	// -------------------------------------------------------
	// get total times array
	// -------------------------------------------------------
	int get_total_times(float* values, int max) {
		int end = ctx.numTimes;
		if (end > max) {
			end = max;
		}
		int cnt = 0;
		for (int i = 0; i < end; ++i) {
			values[cnt++] = ctx.totalTimes[i];
		}
		return cnt;
	}

	// -------------------------------------------------------
	// Logs profiling data
	// -------------------------------------------------------
	void print() {
		LOG << "------------------------------------------------------------";
		LOG << " C  | Percent | Accu       | Name";
		LOG << "------------------------------------------------------------";
		float norm = ctx.data[0].totalTime;
		char buffer[256];
		std::string line;
		char p[10];
		for (int i = 0; i < ctx.index; ++i) {
			ProfileData& pd = ctx.data[i];
			int ident = pd.level * 2;
			float per = pd.totalTime / norm * 100.0f;
			ds::string::formatPercentage(per, p);
			sprintf(buffer,"%3d | %s  | %3.8f | ", pd.invokeCounter, p, pd.totalTime);
			line = buffer;
			for (int j = 0; j < ident; ++j) {
				line += " ";
			}
			LOG << line << " " << pd.name;
		}
		LOG << "------------------------------------------------------------";
	}

	void save(const ds::ReportWriter& writer) {
		writer.addHeader("Profiling");
		char p[10];
		const char* HEADERS[] = { "C", "Percent", "Accu", "Name" };
		writer.startTable(HEADERS, 4);
		float norm = ctx.data[0].totalTime;
		for (int i = 0; i < ctx.index; ++i) {
			writer.startRow();
			ProfileData& pd = ctx.data[i];
			int ident = pd.level * 2;
			float per = pd.totalTime / norm * 100.0f;
			writer.addCell(pd.invokeCounter);
			ds::string::formatPercentage(per, p);
			writer.addCell(p);
			writer.addCell(pd.totalTime);
			writer.addCell(ident,pd.name);
			writer.endRow();
		}
		writer.endTable();
	}
	/*
	void showDialog(v2* position) {
		gui::start(EDITOR_ID, position);
		int state = 1;
		if (gui::begin("Profiler", &state)) {
			float norm = ctx.data[0].totalTime;
			char buffer[256];
			for (int i = 0; i < ctx.index; ++i) {
				ProfileData& pd = ctx.data[i];
				int ident = pd.level * 2;
				//float per = pd.totalTime / norm * 100.0f;
				sprintf(buffer, "%3d - %3.8f - %2.11s", pd.invokeCounter, pd.totalTime,pd.name);
				gui::Label(buffer);
			}
		}
		gui::end();
	}
	*/
	int get_snapshot(ProfileSnapshot* items, int max) {
		int realMax = ctx.index;
		if (realMax > max) {
			realMax = max;
		}
		float total = 0.0f;
		for (int i = 0; i < realMax; ++i) {
			ProfileData& pd = ctx.data[i];
			ProfileSnapshot& snap = items[i];
			strcpy(snap.name, pd.name);
			snap.level = pd.level;
			snap.invokeCounter = pd.invokeCounter;
			snap.totalTime = pd.totalTime;
			total += pd.totalTime;
		}
		items[0].totalTime = total;
		return realMax;
	}
}



namespace perf {

	struct ZoneTrackerEvent {
		IdString hash;
		int parent;
		LARGE_INTEGER started;
		float duration;
		int name_index;
		int ident;
	};

	struct ZoneTrackerContext {
		ds::CharBuffer names;
		ds::Array<IdString> hashes;
		int current_parent;
		int root_event;
		ds::Array<ZoneTrackerEvent> events;
		int ident;
	};

	static ZoneTrackerContext* zoneTrackerCtx;

	void init() {
		zoneTrackerCtx = new ZoneTrackerContext;
	}

	void reset() {
		zoneTrackerCtx->names.size = 0;
		zoneTrackerCtx->events.clear();
		zoneTrackerCtx->ident = 0;
		// create root event
		zoneTrackerCtx->current_parent = -1;
		zoneTrackerCtx->root_event = start("ROOT");		
		zoneTrackerCtx->current_parent = zoneTrackerCtx->root_event;
	}

	void finalize() {
		end(zoneTrackerCtx->root_event);
	}

	void debug() {
		LOG << "------------------------------------------------------------";
		LOG << " Percent | Accu       | Name";
		LOG << "------------------------------------------------------------";
		float norm = zoneTrackerCtx->events[0].duration;
		char buffer[256];
		char p[10];
		std::string line;
		for (int i = 0; i < zoneTrackerCtx->events.size(); ++i) {
			const ZoneTrackerEvent& event = zoneTrackerCtx->events[i];
			int ident = event.ident * 2;
			float per = event.duration / norm * 100.0f;
			ds::string::formatPercentage(per, p);
			sprintf(buffer, "%s  | %3.8f | ", p, event.duration);
			line = buffer;
			for (int j = 0; j < ident; ++j) {
				line += " ";
			}
			const char* n = zoneTrackerCtx->names.data + zoneTrackerCtx->events[i].name_index;
			LOG << line << " " << n;
		}
		LOG << "------------------------------------------------------------";

	}

	int findHash(IdString hash) {
		for (int i = 0; i < zoneTrackerCtx->events.size(); ++i) {
			if (zoneTrackerCtx->events[i].hash == hash) {
				return i;
			}
		}
		return -1;
	}

	int start(const char* name) {
		// create event
		ZoneTrackerEvent event;
		event.parent = zoneTrackerCtx->current_parent;
		QueryPerformanceCounter(&event.started);
		event.ident = zoneTrackerCtx->ident++;
		event.hash = ds::string::murmur_hash(name);
		int idx = findHash(event.hash);
		if (idx == -1) {
			event.name_index = zoneTrackerCtx->names.size;
			int l = strlen(name);
			if (zoneTrackerCtx->names.size + l > zoneTrackerCtx->names.capacity) {
				zoneTrackerCtx->names.resize(zoneTrackerCtx->names.capacity + 256);
			}
			zoneTrackerCtx->names.append(name, l);
		}
		else {
			event.name_index = zoneTrackerCtx->events[idx].name_index;
		}
		int eventIndex = zoneTrackerCtx->events.size();
		zoneTrackerCtx->events.push_back(event);
		zoneTrackerCtx->current_parent = eventIndex;
		return eventIndex;
	}

	void end(int index) {
		ZoneTrackerEvent& event = zoneTrackerCtx->events[index];
		LARGE_INTEGER EndingTime;
		QueryPerformanceCounter(&EndingTime);
		LARGE_INTEGER time;
		time.QuadPart = EndingTime.QuadPart - event.started.QuadPart;
		event.duration = profiler::LIToSecs(time);
		if (zoneTrackerCtx->events[zoneTrackerCtx->current_parent].parent != -1) {
			zoneTrackerCtx->current_parent = zoneTrackerCtx->events[zoneTrackerCtx->current_parent].parent;
		}
		--zoneTrackerCtx->ident;
	}

	void shutdown() {
		delete zoneTrackerCtx;
	}

	struct CallAggregator {

		IdString hash;
		int calls;
		float total;
		int name_index;

		bool operator > (const CallAggregator& other) const {
			return (total > other.total);
		}
	};

	void save(const ds::ReportWriter& writer) {
		writer.startBox("Perf - Profiling");
		const char* HEADERS[] = { "Percent", "Accu", "Name" };
		writer.startTable(HEADERS, 3);
		char p[10];
		float norm = zoneTrackerCtx->events[0].duration;
		for (int i = 0; i < zoneTrackerCtx->events.size(); ++i) {
			writer.startRow();
			const ZoneTrackerEvent& event = zoneTrackerCtx->events[i];
			int ident = event.ident * 2;
			float per = event.duration / norm * 100.0f;
			ds::string::formatPercentage(per, p);
			writer.addCell(p);
			writer.addCell(event.duration);
			const char* n = zoneTrackerCtx->names.data + zoneTrackerCtx->events[i].name_index;
			writer.addCell(event.ident * 2, n);
			writer.endRow();
		}
		writer.endTable();
		writer.endBox();

		std::vector<CallAggregator> calls;
		for (int i = 0; i < zoneTrackerCtx->events.size(); ++i) {
			const ZoneTrackerEvent& event = zoneTrackerCtx->events[i];
			int idx = -1;
			for (size_t j = 0; j < calls.size(); ++j) {
				if (calls[j].hash == event.hash) {
					idx = j;
				}
			}
			if (idx == -1) {
				CallAggregator ac;
				ac.calls = 1;
				ac.hash = event.hash;
				ac.total = event.duration;
				ac.name_index = event.name_index;
				calls.push_back(ac);
			}
			else {
				CallAggregator& ac = calls[idx];
				++ac.calls;
				ac.total += event.duration;
			}
		}
		std::sort(calls.begin(), calls.end(),std::greater<CallAggregator>());
		
		writer.startBox("Perf - Top calls");
		const char* TC_HEADERS[] = { "Calls", "Total", "Name" };
		writer.startTable(TC_HEADERS, 3);
		for (int i = 0; i < calls.size(); ++i) {
			const CallAggregator& ac = calls[i];
			writer.startRow();
			writer.addCell(ac.calls);
			writer.addCell(ac.total);
			const char* n = zoneTrackerCtx->names.data + ac.name_index;
			writer.addCell(n);
			writer.endRow();
		}
		writer.endTable();
		writer.endBox();

	}

	float get_current_total_time() {
		if (zoneTrackerCtx->events.size() > 0) {
			return zoneTrackerCtx->events[0].duration;
		}
		else {
			return 0.0f;
		}
	}
	/*
	void showDialog(v2* position) {
		gui::start(EDITOR_ID, position);
		int state = 1;
		if (gui::begin("Profiler", &state)) {
			char buffer[256];
			float norm = zoneTrackerCtx->events[0].duration;
			for (int i = 0; i < zoneTrackerCtx->events.size(); ++i) {
				const ZoneTrackerEvent& event = zoneTrackerCtx->events[i];
				int ident = pd.level * 2;
				//float per = pd.totalTime / norm * 100.0f;
				sprintf(buffer, "%3.8f - %2.11s", pd.totalTime, pd.name);
				gui::Label(buffer);
			}
		}
		gui::end();
	}

	int get_snapshot(ProfileSnapshot* items, int max) {
		int realMax = ctx.index;
		if (realMax > max) {
			realMax = max;
		}
		float total = 0.0f;
		for (int i = 0; i < realMax; ++i) {
			ProfileData& pd = ctx.data[i];
			ProfileSnapshot& snap = items[i];
			strcpy(snap.name, pd.name);
			snap.level = pd.level;
			snap.invokeCounter = pd.invokeCounter;
			snap.totalTime = pd.totalTime;
			total += pd.totalTime;
		}
		items[0].totalTime = total;
		return realMax;
	}
	*/
}

