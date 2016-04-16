#include "FollowTargetAction.h"
#include "..\..\math\math.h"
#include "..\..\utils\Log.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	FollowTargetAction::FollowTargetAction() : AbstractAction("follow_target") {
		int sizes[] = { sizeof(SID), sizeof(SID), sizeof(float)};
		_buffer.init(sizes, 3);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	FollowTargetAction::~FollowTargetAction() {}

	void FollowTargetAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_targets = (SID*)_buffer.get_ptr(1);
			_velocities = (float*)_buffer.get_ptr(2);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void FollowTargetAction::attach(SID id,SID target,float velocity) {
		int idx = create(id);
		_ids[idx] = id;
		_velocities[idx] = velocity;
		_targets[idx] = target;
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void FollowTargetAction::update(SpriteArray& array, float dt, ActionEventBuffer& buffer) {
		if (_buffer.size > 0) {
			for (int i = 0; i <_buffer.size; ++i) {
				SID targetID = _targets[i];
				if (array.contains(targetID)) {
					v2 p = array.getPosition(_ids[i]);
					v2 targetPos = array.getPosition(targetID);
					v2 diff = targetPos - p;
					if (sqr_length(diff) > 100.0f) {
						v2 n = normalize(diff);
						n *= _velocities[i] * dt;
						p += n;
						array.setPosition(_ids[i], p);
						float angle = math::calculateRotation(n);
						array.rotate(_ids[i], angle);
					}
				}
				else {
					removeByIndex(i);
				}
			}

			//Vector2f v;
			//for (int i = 0; i < _num; ++i) {
				/*
				int in = array.getIndex(_ids[i]);
				int cnt = computeSeparation(array, i, 20.0f, &v);
				if (cnt > 0) {
					Vector2f n = normalize(v);
					array.positions[in] += n * _velocities[in] * dt * 0.75f;
				}
				*/
				/*
				int idx = findNearest(array,array.positions[in], 20.0f, in,array.types[in]);
				if (idx != -1) {					
					Vector2f p = array.positions[idx];
					Vector2f diff = array.positions[in] - p;
					if (sqr_length(diff) < 400.0f) {
						Vector2f n = normalize(diff);
						//n *= 0.5f;
						n *= _velocities[i] * dt;
						array.positions[in] += n;
						//n = normalize(diff);
						//n *= -1.0f * _velocities[idx] * dt;
						array.positions[idx] -= n;
					}
				}
				*/
			//}
			
		}
	}

	// -------------------------------------------------------
	// find nearest enemy to current enemy
	// -------------------------------------------------------
	int FollowTargetAction::findNearest(SpriteArray& array,const Vector2f& pos, float radius, int self,int type) {
		int ret = -1;
		float min = 2000.0f * 2000.0f;
		for (int i = 0; i < array.num; ++i) {
			if (i != self && array.types[i] == type) {
				Vector2f diff = pos - array.positions[i];
				float distSq = sqr_length(diff);
				if (distSq < (radius + radius)*(radius + radius)) {
					if (distSq < min) {
						ret = i;
						min = distSq;
					}
				}
			}
		}
		return ret;
	}

	// -------------------------------------------------------
	// find nearest enemy to current enemy
	// -------------------------------------------------------
	int FollowTargetAction::computeSeparation(SpriteArray& array, int self, float radius,Vector2f* v) {
		int count = 0;
		v->x = 0.0f;
		v->y = 0.0f;
		int type = array.types[self];
		Vector2f pos = array.positions[self];
		for (int i = 0; i < array.num; ++i) {
			if (i != self && array.types[i] == type) {
				Vector2f diff = pos - array.positions[i];
				//float distSq = sqr_length(diff);
				if (sqr_length(diff) < (radius + radius)*(radius + radius)) {
					*v += diff;
					++count;
				}
			}
		}
		*v *= -1.0f;
		if (count > 0) {
			v->x /= count;
			v->y /= count;
		}
		return count;
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void FollowTargetAction::debug() {
		for (int i = 0; i < _buffer.size; ++i) {
			LOG << i << " : id: " << _ids[i] << " velocity: " << _velocities[i];
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