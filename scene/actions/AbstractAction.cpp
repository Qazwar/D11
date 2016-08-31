#include "AbstractAction.h"
#include "core\log\Log.h"

namespace ds {

	void AbstractAction::removeByIndex(int i) {
		swap(i);
	}

	void AbstractAction::setBoundingRect(const Rect& r) {
		m_BoundingRect = r;
	}

	int AbstractAction::find(ID id) {
		for (int i = 0; i < _buffer.size; ++i) {
			if (_ids[i] == id) {
				return i;
			}
		}
		return -1;
	}

	bool AbstractAction::contains(ID id) {
		return find(id) != -1;
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	ID AbstractAction::swap(int i) {
		ID current = _ids[i];
		_buffer.remove(i);
		return current;
	}

	// -------------------------------------------------------
	// create new entry
	// -------------------------------------------------------
	int AbstractAction::create(ID id) {
		int idx = find(id);
		if (idx == -1) {
			allocate(_buffer.size + 16);
			idx = _buffer.size;
			++_buffer.size;
		}
		return idx;
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void AbstractAction::clear() {
		_buffer.size = 0;
	}

	void AbstractAction::removeByID(ID id) {
		int idx = find(id);
		if (idx != -1) {
			swap(idx);
		}
	}
}
