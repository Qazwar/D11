#include "TextureParser.h"

namespace ds {

	namespace res {

		RID TextureParser::loadTexture(const char* name, const TextureDescriptor& descriptor) {
			ID3D11ShaderResourceView* srv = 0;
			char buffer[256];
			sprintf_s(buffer, 256, "content\\textures\\%s", descriptor.name);
			ds::File f(SID(buffer), FileType::FT_BINARY);
			if (repository::load(&f) != ds::FileStatus::FS_OK) {
				LOGE << "Cannot load texture file";
			}
			HRESULT d3dResult = D3DX11CreateShaderResourceViewFromMemory(_resCtx->device, f.data, f.size, 0, 0, &srv, 0);
			if (FAILED(d3dResult)) {
				DXTRACE_MSG("Failed to load the texture image!");
				return INVALID_RID;
			}
			ShaderResourceViewResource* cbr = new ShaderResourceViewResource(srv);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::TEXTURE);
		}

		RID TextureParser::parse(JSONReader& reader, int childIndex) {
			TextureDescriptor descriptor;
			descriptor.name = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			return loadTexture(name, descriptor);
		}
	}
}