#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include "..\EntityArray.h"
#include "..\..\math\tweening.h"
#include "..\..\lib\BlockArray.h"
#include "..\..\io\ReportWriter.h"
#include "..\..\utils\StaticHash.h"

namespace ds {
	/*
	struct AbstractActionDefinition {

		virtual void read(const JSONReader& reader, int category_id) = 0;
		virtual ActionType getActionType() const = 0;
	};
	*/
	class AbstractAction {

		public:
			AbstractAction(const char* name) : _name(name) {
				_hash = SID(name);
			}
			virtual ~AbstractAction() {}
			virtual void update(EntityArray& array,float dt,ActionEventBuffer& buffer) = 0;
			void removeByIndex(int i);
			virtual void debug() = 0;
			virtual void debug(ID sid) = 0;
			void setBoundingRect(const Rect& r);
			virtual void allocate(int sz) = 0;
			virtual ActionType getActionType() const = 0;
			//virtual void attach(ID id, AbstractActionDefinition* definition) {}// = 0;
			void clear();
			void removeByID(ID id);
			bool contains(ID id);
			virtual void save(const ReportWriter& writer) {}
			const char* getName() const {
				return _name;
			}
			const StaticHash& getHash() const {
				return _hash;
			}
		protected:
			int create(ID id);
			int find(ID id);
			ID swap(int index);
			Rect m_BoundingRect;
			BlockArray _buffer;
			ID* _ids;
		private:
			const char* _name;
			StaticHash _hash;
		};

	

}
