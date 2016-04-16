#include "..\io\DataFile.h"


namespace ds {

	struct AbstractActionDefinition;

	struct Behavior {
		IdString hash;
		int definitions[32];
		int num;
	};

	class World;

	class BehaviorDefinitions : public DataFile {

	public:
		BehaviorDefinitions(World* world) : _world(world) {}
		~BehaviorDefinitions() {
			clear();
		}
		bool saveData(JSONWriter& writer);
		bool loadData(const JSONReader& loader);
		Behavior* get(const char* name);
		const char* getFileName() const {
			return "resources\\behaviors.json";
		}
		AbstractActionDefinition* getDefinition(int index) {
			return _definitions[index];
		}
	private:
		void clear();
		AbstractActionDefinition* createDefinition(const char* name, int* index);
		Array<Behavior> _behaviors;
		Array<AbstractActionDefinition*> _definitions;
		World* _world;
	};

}