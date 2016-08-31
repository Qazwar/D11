#include "MeshGenCommand.h"
#include "..\imgui\IMGUI.h"
#include "core\math\math.h"

namespace ds {

	namespace gen {

		bool AddCubeCommand::draw() {
			bool ret = false;
			if (isActive()) {
				gui::Header("Add cube");
				gui::InputVec3("Pos", &_pos);
				gui::InputVec3("Size", &_scale);
				gui::beginGroup();
				if (gui::Button("Add")) {
					_context->gen->add_cube(_pos, _scale);
					for (int i = 0; i < 128; ++i) {
						_context->gen->texture_face(i, math::buildTexture(682, 260, 32, 32));
					}
					setActive(false);
					ret = true;
				}
				if (gui::Button("Cancel")) {
					setActive(false);
				}
				gui::endGroup();
			}
			return ret;
		}

		bool ExtrudeEdgeCommand::draw() {
			bool ret = false;
			if (isActive()) {
				gui::Header("Extrude edge");
				gui::InputInt("Index", &_edge);
				gui::InputVec3("Dir", &_direction);
				gui::beginGroup();
				if (gui::Button("Extrude")) {
					_context->gen->extrude_edge(_edge, _direction);
					ret = true;
					setActive(false);
				}
				if (gui::Button("Cancel")) {
					setActive(false);
				}
				gui::endGroup();
			}
			return ret;
		}

		bool SetColorCommand::draw() {
			bool ret = false;
			if (isActive()) {
				gui::Header("Set color");
				gui::InputColor("Color", &_color);
				gui::beginGroup();
				if (gui::Button("Apply")) {
					_context->gen->set_color(_color);
					ret = true;
					setActive(false);
				}
				if (gui::Button("Cancel")) {
					setActive(false);
				}
				gui::endGroup();
			}
			return ret;
		}
	}
}