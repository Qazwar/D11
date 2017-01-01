#pragma once

namespace ds {

	// --------------------------------------------------
	// Virtual base class for all editor plugins
	// --------------------------------------------------
	class EditorPlugin {

	public:
		EditorPlugin(const char* name) : _name(name) , _active(false) {}
		virtual ~EditorPlugin() {}
		virtual void showDialog() = 0;
		const char* getName() const {
			return _name;
		}
		const bool isActive() const {
			return _active;
		}
		void activate() {
			_active = true;
		}
		void deactivate() {
			_active = false;
		}
	private:
		const char* _name;
		bool _active;
	};

}