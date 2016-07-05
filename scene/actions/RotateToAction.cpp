#include "RotateToAction.h"
#include "..\..\utils\Log.h"
#include "..\..\math\GameMath.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	RotateToAction::RotateToAction() : AbstractAction("rotate_to") {
		int sizes[] = { sizeof(ID), sizeof(v3), sizeof(v3), sizeof(float), sizeof(float), sizeof(tweening::TweeningType), sizeof(int) };
		_buffer.init(sizes, 7);
	}

	void RotateToAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (ID*)_buffer.get_ptr(0);
			_startAngles  = (v3*)_buffer.get_ptr(1);
			_endAngles = (v3*)_buffer.get_ptr(2);
			_timers = (float*)_buffer.get_ptr(3);
			_ttl = (float*)_buffer.get_ptr(4);
			_tweeningTypes = (tweening::TweeningType*)_buffer.get_ptr(5);
			_modes = (int*)_buffer.get_ptr(6);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void RotateToAction::attach(SID id, const v3& startAngle, const v3& endAngle, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		int idx = create(id);
		_ids[idx] = id;
		_startAngles[idx] = startAngle;
		_endAngles[idx] = endAngle;
		_timers[idx] = 0.0f;
		_ttl[idx] = ttl;
		_tweeningTypes[idx] = tweeningType;
		_modes[idx] = mode;
		if ( mode > 0 ) {
			--_modes[idx];
		}
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void RotateToAction::update(EntityArray& array, float dt, ActionEventBuffer& buffer) {
		if (_buffer.size > 0) {
			// move
			for (int i = 0; i < _buffer.size; ++i) {
				int ai = array.getIndex(_ids[i]);
				array.rotations[ai] = tweening::interpolate(_tweeningTypes[i], _startAngles[i], _endAngles[i], _timers[i], _ttl[i]);
				array.dirty[ai] = true;
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						array.rotations[ai] = tweening::interpolate(_tweeningTypes[i], _startAngles[i], _endAngles[i], _ttl[i], _ttl[i]);
						array.dirty[ai] = true;
						buffer.add(_ids[i], AT_ROTATE_TO, array.types[ai]);
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
	void RotateToAction::debug() {
		for (int i = 0; i < _buffer.size; ++i) {
			LOG << i << " : id: " << _ids[i] << " start: " << _startAngles[i] << " end: " << DBG_V3(_endAngles[i]) << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

	void RotateToAction::debug(ID sid) {
		int i = find(sid);
		if (i != -1) {
			LOG << "RotateAction - id: " << _ids[i] << " start: " << _startAngles[i] << " end: " << DBG_V3(_endAngles[i]) << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

	// -------------------------------------------------------
	// save to report
	// -------------------------------------------------------
	void RotateToAction::save(const ReportWriter& writer) {
		writer.addHeader("RotateToAction");
		const char* HEADERS[] = { "ID", "Start", "End", "Timer", "TTL" };
		writer.startTable(HEADERS, 5);
		for (uint32_t i = 0; i < _buffer.size; ++i) {
			writer.startRow();
			writer.addCell(_ids[i]);
			writer.addCell(_startAngles[i]);
			writer.addCell(_endAngles[i]);
			writer.addCell(_timers[i]);
			writer.addCell(_ttl[i]);
			writer.endRow();
		}
		writer.endTable();
	}

}