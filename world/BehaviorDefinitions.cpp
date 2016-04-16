#include "BehaviorDefinitions.h"
#include "actions\ColorFlashAction.h"
#include "actions\AbstractAction.h"
#include "World.h"

namespace ds {

	void BehaviorDefinitions::clear() {
		_definitions.destroy_all();
	}
	// -----------------------------------------------------
	// save data
	// -----------------------------------------------------
	bool BehaviorDefinitions::saveData(JSONWriter& writer) {
		return true;
	}

	// -----------------------------------------------------
	// load data
	// -----------------------------------------------------
	bool BehaviorDefinitions::loadData(const JSONReader& loader) {
		clear();
		int cats[256];
		int num = loader.get_categories(cats, 256);
		int index = 0;
		for (int i = 0; i < num; ++i) {
			// create Behavior
			Behavior b;
			b.num = 0;
			b.hash = string::murmur_hash(loader.get_category_name(cats[i]));
			int sub_cats[64];
			int sub_num = loader.get_categories(sub_cats, 64, cats[i]);
			for (int j = 0; j < sub_num; ++j) {
				// find action definition
				AbstractActionDefinition* def = createDefinition(loader.get_category_name(sub_cats[j]),&index);
				if (def != 0) {
					def->read(loader, sub_cats[j]);
					b.definitions[b.num++] = index;
				}
			}
			if (b.num > 0) {
				_behaviors.push_back(b);
			}
		}
		return true;
	}

	// -----------------------------------------------------
	// create definition
	// -----------------------------------------------------
	AbstractActionDefinition* BehaviorDefinitions::createDefinition(const char* name,int* index) {
		AbstractAction* action = _world->find_action(name);
		if (action != 0) {
			*index = _definitions.size();
			AbstractActionDefinition* def = action->createDefinition();			
			if (def != 0) {
				_definitions.push_back(def);
				return def;
			}
		}
		return 0;
	}

	Behavior* BehaviorDefinitions::get(const char* name) {
		IdString hash = string::murmur_hash(name);
		for (size_t i = 0; i < _behaviors.size(); ++i) {
			if (_behaviors[i].hash == hash) {
				return &_behaviors[i];
			}
		}
		return 0;
	}

}