#pragma once
#include "..\renderer\MeshBuffer.h"
#include "MeshGen.h"

namespace ds {

	namespace gen {

		enum GenSelectionType {
			VERTEX,
			EDGE,
			FACE,
			OBJECT
		};

		struct CommandContext {

			ds::gen::MeshGen* gen;
			ds::Mesh* mesh;
			GenSelectionType selectionType;
			ds::Array<uint16_t> selections;
			int selectedFace;
			ds::Array<float> data;
		};

		class MeshCommand {

		public:
			MeshCommand(CommandContext* context) : _context(context), _active(false) {}
			virtual ~MeshCommand() {}
			virtual bool draw() = 0;
			virtual const char* getButtonName() const = 0;
			const bool isActive() const {
				return _active;
			}
			void setActive(bool active) {
				_active = active;
			}
		protected:
			CommandContext* _context;
		private:
			bool _active;
		};

		class AddCubeCommand : public MeshCommand {

		public:
			AddCubeCommand(CommandContext* context) : MeshCommand(context), _pos(0, 0, 0), _scale(1, 1, 1) {}
			virtual ~AddCubeCommand() {}
			bool draw();
			const char* getButtonName() const {
				return "Add Cube";
			}
		private:
			v3 _pos;
			v3 _scale;
		};

		class ExtrudeEdgeCommand : public MeshCommand {

		public:
			ExtrudeEdgeCommand(CommandContext* context) : MeshCommand(context), _direction(1, 0, 0), _edge(-1) {}
			virtual ~ExtrudeEdgeCommand() {}
			bool draw();
			const char* getButtonName() const {
				return "Ext_edge";
			}
		private:
			v3 _direction;
			int _edge;
		};

		class SetColorCommand : public MeshCommand {

		public:
			SetColorCommand(CommandContext* context) : MeshCommand(context), _color(ds::Color::WHITE) {}
			virtual ~SetColorCommand() {}
			bool draw();
			const char* getButtonName() const {
				return "set_clr";
			}
		private:
			ds::Color _color;
		};
	}
}