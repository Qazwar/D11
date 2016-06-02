#pragma once
#include "MeshGenCommand.h"
#include "..\renderer\Camera.h"

namespace ds {

	namespace gen {

		class MeshGenGUI {

		public:
			MeshGenGUI(const char* orthoCameraName,CommandContext* ctx);
			~MeshGenGUI();
			void drawGUI();
			void handleClick(Camera* camera);
			void rebuildMesh();
		private:
			CommandContext* _ctx;
			ds::Array<MeshCommand*> _commands;
			int _selectedFace;
			OrthoCamera* _orthoCamera;
			bool _pressed;
			Color _prevColor;
		};

	}
}