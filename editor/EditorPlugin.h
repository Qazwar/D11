#pragma once
#include <core\base\StateObject.h>

namespace ds {

	// --------------------------------------------------
	// Virtual base class for all editor plugins
	// --------------------------------------------------
	class EditorPlugin : public StateObject {

	public:
		EditorPlugin(const char* name) : StateObject() , _name(name) {}
		virtual ~EditorPlugin() {}
		virtual void showDialog() = 0;
		const char* getName() const {
			return _name;
		}
	private:
		const char* _name;
	};

}