#include "FollowStraightPathAction.h"
#include "..\..\math\math.h"
#include "..\..\utils\Log.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	FollowStraightPathAction::FollowStraightPathAction() : AbstractAction("follow_straight_path") {
		int sizes[] = { sizeof(SID), sizeof(StraightPath*), sizeof(float),sizeof(float), sizeof(int) };
		_buffer.init(sizes, 5);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	FollowStraightPathAction::~FollowStraightPathAction() {}

	void FollowStraightPathAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_path = (StraightPath**)_buffer.get_ptr(1);
			_timers = (float*)_buffer.get_ptr(2);
			_ttl = (float*)_buffer.get_ptr(3);
			_modes = (int*)_buffer.get_ptr(4);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void FollowStraightPathAction::attach(SID id, SpriteArray& array, StraightPath* path, float ttl, int mode) {
		int idx = create(id);
		_ids[idx] = id;
		_path[idx] = path;
		_timers[idx] = 0.0f;
		_ttl[idx] = ttl;
		_modes[idx] = mode;
		if ( mode > 0 ) {
			--_modes[idx];
		}
		Vector2f p;
		path->approx(0.0f, &p);
		array.setPosition(id, p);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void FollowStraightPathAction::update(SpriteArray& array, float dt, ActionEventBuffer& buffer) {
		if (_buffer.size > 0) {
			// move
			Vector2f p;
			Vector2f t;
			for (int i = 0; i < _buffer.size; ++i) {
				float norm = math::norm(_timers[i] , _ttl[i]);
				_path[i]->approx(norm,&p);

				_path[i]->tanget(norm, &t);
				float a = math::calculateRotation(t);
				array.rotate(_ids[i], a);
				array.setPosition(_ids[i],p);
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						_path[i]->get(1.0f,&p);
						array.setPosition(_ids[i],p);						
						buffer.add(_ids[i], AT_FOLLOW_STRAIGHT_PATH, array.getType(_ids[i]));
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
	void FollowStraightPathAction::debug() {
		for (int i = 0; i < _buffer.size; ++i) {
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

}