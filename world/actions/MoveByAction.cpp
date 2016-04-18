#include "MoveByAction.h"
#include "..\..\utils\Log.h"
#include "..\..\math\math.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	MoveByAction::MoveByAction() : AbstractAction("move_by") {
		int sizes[] = { sizeof(SID), sizeof(v2), sizeof(bool) };
		_buffer.init(sizes, 3);
	}

	void MoveByAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			LOG << "buffer resizing by: " << sz << " to: " << _buffer.capacity;
			_ids = (SID*)_buffer.get_ptr(0);
			_velocities = (v2*)_buffer.get_ptr(1);
			_bounce = (bool*)_buffer.get_ptr(2);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveByAction::attach(SID id,const Vector2f& velocity,bool bounce) {		
		int idx = create(id);
		_ids[idx] = id;
		_velocities[idx] = velocity;
		_bounce[idx] = bounce;
	}

	void MoveByAction::rotateTo(SpriteArray& array, int index) {
		float angle = math::calculateRotation(_velocities[index]);
		array.rotate(_ids[index], angle);
	}

	bool MoveByAction::isOutOfBounds(const Vector2f& pos, const Vector2f& v) {
		if (v.x > 0.0f && pos.x > m_BoundingRect.right) {
			return true;
		}
		if (v.x < 0.0f && pos.x < m_BoundingRect.left) {
			return true;
		}
		if (v.y > 0.0f && pos.y > m_BoundingRect.top) {
			return true;
		}
		if (v.y < 0.0f && pos.y < m_BoundingRect.bottom) {
			return true;
		}
		return false;
	}

	void MoveByAction::bounce(SpriteArray& array, SID sid, BounceDirection direction,float dt) {
		for (int i = 0; i < _buffer.size; ++i) {
			if (_ids[i] == sid) {
				if (direction == BD_Y || direction == BD_BOTH) {
					_velocities[i].y *= -1.0f;
				}
				if (direction == BD_X || direction == BD_BOTH) {
					_velocities[i].x *= -1.0f;
				}
				float angle = math::calculateRotation(_velocities[i]);
				array.rotate(_ids[i], angle);
				Vector2f p = array.getPosition(_ids[i]);
				p += _velocities[i] * dt;
				array.setPosition(_ids[i], p);
			}
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveByAction::update(SpriteArray& array,float dt,ActionEventBuffer& buffer) {	
		if (_buffer.size > 0) {
			for (int i = 0; i < _buffer.size; ++i) {
				Vector2f p = array.getPosition(_ids[i]);
				p += _velocities[i] * dt;
				if (isOutOfBounds(p, _velocities[i])) {
					if (_bounce[i]) {
						if (p.y < m_BoundingRect.bottom || p.y > m_BoundingRect.top) {
							_velocities[i].y *= -1.0f;
						}
						if (p.x < m_BoundingRect.left || p.x > m_BoundingRect.right ) {
							_velocities[i].x *= -1.0f;
						}
						rotateTo(array, i);
						p += _velocities[i] * dt * 1.5f;
					}
					else {
						buffer.add(_ids[i], AT_MOVE_BY, array.getType(_ids[i]));
					}
				}
				array.setPosition(_ids[i], p);
			}
		}
	}
	
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveByAction::debug() {
		LOG << "------- MoveByAction -------";
		for (int i = 0; i < _buffer.size; ++i) {
			LOG << i << " id: " << _ids[i] << " velocity: " << DBG_V2(_velocities[i]);
		}		
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveByAction::debug(SID sid) {
		int i = find(sid);
		if (i != -1) {
			LOG << "MoveByAction - id: " << _ids[i] << " velocity: " << DBG_V2(_velocities[i]);
		}
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveByAction::save(const ReportWriter& writer) {
		if (_buffer.size > 0) {
			writer.addSubHeader("MoveByAction");
			const char* HEADERS[] = { "Index", "ID", "Velocity", "Bounce"};
			writer.startTable(HEADERS, 4);
			for (int i = 0; i < _buffer.size; ++i) {
				writer.startRow();
				writer.addCell(i);
				writer.addCell(_ids[i]);
				writer.addCell(_velocities[i]);
				writer.addCell(_bounce[i]);
				writer.endRow();
			}
			writer.endTable();
		}
	}

}