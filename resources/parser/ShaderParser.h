#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class ShaderParser : public ResourceParser {

		public:
			ShaderParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID createShader(const char* name, const ShaderDescriptor& descriptor);
		private:
			bool createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader);
			bool createGeometryShader(ID3DBlob* buffer, ID3D11GeometryShader** shader);
			bool createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader);
			bool compileShader(const char* filePath, const  char* entry, const  char* shaderModel, ID3DBlob** buffer);
		};

	}

}