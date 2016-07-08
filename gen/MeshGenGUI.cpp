#include "MeshGenGUI.h"
#include "..\imgui\IMGUI.h"
#include "..\resources\ResourceContainer.h"
#include "..\base\InputSystem.h"

namespace ds {

	namespace gen {

		MeshGenGUI::MeshGenGUI(const char* orthoCameraName, CommandContext* ctx) : _ctx(ctx) , _pressed(false) , _prevColor(Color::WHITE) {
			_orthoCamera = graphics::getOrthoCamera();
			_commands.push_back(new AddCubeCommand(_ctx));
			_commands.push_back(new ExtrudeEdgeCommand(_ctx));
			_commands.push_back(new SetColorCommand(_ctx));
		}

		MeshGenGUI::~MeshGenGUI() {
			for (int i = 0; i < _commands.size(); ++i) {
				delete _commands[i];
			}
		}

		void MeshGenGUI::drawGUI() {
			graphics::setCamera(_orthoCamera);
			graphics::turnOffZBuffer();
			v2 pos(10, 760);
			int state = 1;
			gui::start(1, &pos, true);
			gui::begin("MeshGen", &state);
			for (int i = 0; i < _commands.size(); ++i) {
				ds::gen::MeshCommand* cmd = _commands[i];
				if (cmd->isActive()) {
					if (cmd->draw()) {
						rebuildMesh();
					}
				}
				else {
					if (gui::Button(cmd->getButtonName())) {
						cmd->setActive(true);
					}
				}
			}
			if (gui::Button("Clear")) {
				_ctx->gen->clear();
				_ctx->mesh->clear();
			}
			gui::end();
		}

		void MeshGenGUI::rebuildMesh() {
			_ctx->mesh->clear();
			_ctx->gen->build(_ctx->mesh);
		}

		void MeshGenGUI::handleClick(Camera* camera) {
			if (input::isMouseButtonPressed(0) && !_pressed) {
				_pressed = true;
				ds::Ray r = graphics::getCameraRay(camera);
				int selection = _ctx->gen->intersects(r);
				LOG << "selection: " << selection;
				if (selection != -1) {
					_ctx->gen->select_face(selection);
					rebuildMesh();
					_ctx->gen->debug_face(selection);
				}
			}
			if (!input::isMouseButtonPressed(0) && _pressed) {
				_pressed = false;
			}
		}
	}
}