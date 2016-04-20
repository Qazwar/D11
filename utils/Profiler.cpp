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

namespace perf {

	double LIToSecs(LARGE_INTEGER & L, LARGE_INTEGER frequency) {
		return ((double)L.QuadPart * 1000.0 / (double)frequency.QuadPart);
	}

	struct ZoneTrackerEvent {
		IdString hash;
		int parent;
		LARGE_INTEGER started;
		float duration;
		int name_index;
		int ident;
	};

	struct ZoneTrackerContext {
		LARGE_INTEGER frequency;
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
		QueryPerformanceFrequency(&zoneTrackerCtx->frequency);
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
		event.duration = LIToSecs(time, zoneTrackerCtx->frequency);
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

