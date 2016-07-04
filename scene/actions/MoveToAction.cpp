#include "MoveToAction.h"
#include "..\..\utils\Log.h"
#include "..\..\math\GameMath.h"

namespace ds {

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	MoveToAction::MoveToAction() : AbstractAction("move_to") {
		int sizes[] = { sizeof(ID), sizeof(v3), sizeof(v3), sizeof(float), sizeof(float), sizeof(tweening::TweeningType), sizeof(int) };
		_buffer.init(sizes, 7);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveToAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (ID*)_buffer.get_ptr(0);
			_startPositions = (v3*)_buffer.get_ptr(1);
			_endPositions = (v3*)_buffer.get_ptr(2);
			_timers = (float*)_buffer.get_ptr(3);
			_ttl = (float*)_buffer.get_ptr(4);
			_tweeningTypes = (tweening::TweeningType*)_buffer.get_ptr(5);
			_modes = (int*)_buffer.get_ptr(6);
		}
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveToAction::attach(ID id, EntityArray& array,const v3& startPos, const v3& endPos, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		int idx = create(id);
		_ids[idx] = id;
		_startPositions[idx] = startPos;
		_endPositions[idx] = endPos;
		_timers[idx] = 0.0f;
		_ttl[idx] = ttl;
		_tweeningTypes[idx] = tweeningType;
		_modes[idx] = mode;
		if ( mode > 0 ) {
			--_modes[idx];
		}
		int ai = array.getIndex(id);
		array.positions[ai] = startPos;
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveToAction::update(EntityArray& array,float dt,ActionEventBuffer& buffer) {	
		if (_buffer.size > 0) {
			// move
			for (int i = 0; i < _buffer.size; ++i) {
				int ai = array.getIndex(_ids[i]);
				array.positions[ai] = tweening::interpolate(_tweeningTypes[i], _startPositions[i], _endPositions[i], _timers[i], _ttl[i]);
				array.dirty[ai] = true;
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						array.positions[ai] = tweening::interpolate(_tweeningTypes[i], _startPositions[i], _endPositions[i], _ttl[i], _ttl[i]);
						buffer.add(_ids[i], AT_MOVE_TO, array.types[ai]);
						removeByIndex(i);
					}
					else {
						--_modes[i];
						_timers[i] = 0.0f;

					}
				}
			}
		}
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveToAction::debug() {
		LOG << "------- MoveToAction -------";
		for (int i = 0; i < _buffer.size; ++i) {
			LOG << i << " id: " << _ids[i] << " start: " << DBG_V3(_startPositions[i]) << " end: " << DBG_V3(_endPositions[i]) << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}		
	}

	void MoveToAction::debug(ID sid) {
		int i = find(sid);
		if (i != -1) {
			LOG << "> move_to : id: " << _ids[i] << " start: " << DBG_V3(_startPositions[i]) << " end: " << DBG_V3(_endPositions[i]) << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

	// -------------------------------------------------------
	// save to report
	// -------------------------------------------------------
	void MoveToAction::save(const ReportWriter& writer) {
		writer.addHeader("MoveToAction");
		const char* HEADERS[] = { "ID", "Start", "End", "Timer", "TTL" };
		writer.startTable(HEADERS, 5);
		for (uint32_t i = 0; i < _buffer.size; ++i) {
			writer.startRow();
			writer.addCell(_ids[i]);
			writer.addCell(_startPositions[i]);
			writer.addCell(_endPositions[i]);
			writer.addCell(_timers[i]);
			writer.addCell(_ttl[i]);
			writer.endRow();
		}
		writer.endTable();
	}

}