#include "ScalingAction.h"
#include "..\..\math\GameMath.h"
#include "..\..\utils\Log.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	ScalingAction::ScalingAction() : AbstractAction("scale") {
		int sizes[] = { sizeof(ID), sizeof(v3), sizeof(v3), sizeof(float), sizeof(float), sizeof(tweening::TweeningType), sizeof(int) };
		_buffer.init(sizes, 7);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	ScalingAction::~ScalingAction() {}

	// -------------------------------------------------------
	// allocate
	// -------------------------------------------------------
	void ScalingAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (ID*)_buffer.get_ptr(0);
			_startScale = (v3*)_buffer.get_ptr(1);
			_endScale = (v3*)_buffer.get_ptr(2);
			_timers = (float*)_buffer.get_ptr(3);
			_ttl = (float*)_buffer.get_ptr(4);
			_tweeningTypes = (tweening::TweeningType*)_buffer.get_ptr(5);
			_modes = (int*)_buffer.get_ptr(6);
		}
	}
	// -------------------------------------------------------
	// attach
	// -------------------------------------------------------
	void ScalingAction::attach(ID id,const v3& startScale,const v3& endScale,float ttl,int mode,const tweening::TweeningType& tweeningType) {
		int idx = create(id);
		_ids[idx] = id;
		_startScale[idx] = startScale;
		_endScale[idx] = endScale;
		_timers[idx] = 0.0f;
		_ttl[idx] = ttl;
		_tweeningTypes[idx] = tweeningType;
		_modes[idx] = mode;
		if ( mode > 0 ) {
			--_modes[idx];
		}
	}

	// -------------------------------------------------------
	// update
	// -------------------------------------------------------
	void ScalingAction::update(EntityArray& array,float dt,ActionEventBuffer& buffer) {
		if (_buffer.size > 0) {
			for (int i = 0; i < _buffer.size; ++i) {
				int idx = array.getIndex(_ids[i]);
				array.scales[idx] = tweening::interpolate(_tweeningTypes[i], _startScale[i], _endScale[i], _timers[i], _ttl[i]);
				array.dirty[idx] = true;
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						int idx = array.getIndex(_ids[i]);
						array.scales[idx] = tweening::interpolate(_tweeningTypes[i], _startScale[i], _endScale[i], _ttl[i], _ttl[i]);
						buffer.add(_ids[i], AT_SCALE, array.types[idx]);
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
	// debug
	// -------------------------------------------------------
	void ScalingAction::debug() {
		if ( _buffer.size > 0 ) {
			LOG << "---------- ScalingAction ---------- ";
		}
		for ( int i = 0; i < _buffer.size; ++i ) {
			LOG << i << " : id: " << _ids[i] << " timer: " << _timers[i];
		}
		/*
		std::map<SID,int>::iterator it = m_Mapping.begin();
		while ( it != m_Mapping.end()) {
			LOG << it->first << " = " << it->second;
			++it;
		}
		*/
	}

	void ScalingAction::save(const ReportWriter& writer) {
		writer.addHeader("ScalingAction");
		const char* HEADERS[] = { "ID", "Start", "End", "Timer", "TTL" };
		writer.startTable(HEADERS, 5);
		for (uint32_t i = 0; i < _buffer.size; ++i) {
			writer.startRow();
			writer.addCell(_ids[i]);
			writer.addCell(_startScale[i]);
			writer.addCell(_endScale[i]);
			writer.addCell(_timers[i]);
			writer.addCell(_ttl[i]);
			writer.endRow();
		}
		writer.endTable();
	}

}