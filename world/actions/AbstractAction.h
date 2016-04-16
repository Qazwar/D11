#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include "..\World.h"
#include "..\..\math\tweening.h"
#include "..\..\lib\BlockArray.h"
#include "..\..\io\ReportWriter.h"

namespace ds {

	struct AbstractActionDefinition {

		virtual void read(const JSONReader& reader, int category_id) = 0;
		virtual ActionType getActionType() const = 0;
	};

	class AbstractAction {

		public:
			AbstractAction(const char* name) : _name(name) {
				_hash = string::murmur_hash(name);
			}
			virtual ~AbstractAction() {}
			virtual void update(SpriteArray& array,float dt,ActionEventBuffer& buffer) = 0;
			void removeByIndex(int i);
			virtual void debug() = 0;
			virtual void debug(SID sid) = 0;
			void setBoundingRect(const Rect& r);
			virtual void allocate(int sz) = 0;
			virtual ActionType getActionType() const = 0;
			virtual void attach(SID id, AbstractActionDefinition* definition) {}// = 0;
			void clear();
			void removeByID(SID id);
			bool contains(SID id);
			virtual void save(const ReportWriter& writer) {}
			const char* getName() const {
				return _name;
			}
			IdString getHash() const {
				return _hash;
			}
			virtual AbstractActionDefinition* createDefinition() const {
				return 0;
			}
		protected:
			int create(SID id);
			int find(SID id);
			SID swap(int index);
			Rect m_BoundingRect;
			BlockArray _buffer;
			SID* _ids;
		private:
			const char* _name;
			IdString _hash;
		};

	

}
