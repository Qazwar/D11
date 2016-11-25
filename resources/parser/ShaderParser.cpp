#include "ShaderParser.h"
#include <core\base\Assert.h>
#include <D3D11.h>
#include <D3Dcompiler.h>

namespace ds {

	namespace res {

		bool ShaderParser::createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader) {
			XASSERT(buffer != 0, "The buffer must not be NULL");
			HRESULT d3dResult = _resCtx->device->CreateVertexShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, shader);
			if (d3dResult < 0) {
				if (buffer) {
					buffer->Release();
				}
				return false;
			}
			return true;
		}

		bool ShaderParser::createGeometryShader(ID3DBlob* buffer, ID3D11GeometryShader** shader) {
			XASSERT(buffer != 0, "The buffer must not be NULL");
			HRESULT d3dResult = _resCtx->device->CreateGeometryShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, shader);
			if (d3dResult < 0) {
				if (buffer) {
					buffer->Release();
				}
				return false;
			}
			return true;
		}

		bool ShaderParser::createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader) {
			XASSERT(buffer != 0, "The buffer must not be NULL");
			HRESULT d3dResult = _resCtx->device->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, shader);
			if (d3dResult < 0) {
				if (buffer) {
					buffer->Release();
				}
				return false;
			}
			return true;
		}

		bool ShaderParser::compileShader(const char* filePath, const  char* entry, const  char* shaderModel, ID3DBlob** buffer) {
			DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
			shaderFlags |= D3DCOMPILE_DEBUG;
#endif

			ID3DBlob* errorBuffer = 0;
			HRESULT result;

			result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel, shaderFlags, 0, 0, buffer, &errorBuffer, 0);

			if (FAILED(result))	{
				if (errorBuffer != 0) {
					LOGE << "Error compiling shader: " << (char*)errorBuffer->GetBufferPointer();
					errorBuffer->Release();
				}
				return false;
			}
			if (errorBuffer != 0) {
				errorBuffer->Release();
			}
			return true;
		}


		RID ShaderParser::createShader(const char* name, const ShaderDescriptor& descriptor) {
			Shader* s = new Shader;
			s->vertexShaderBuffer = 0;
			bool compileResult = compileShader(descriptor.file, descriptor.vertexShader, "vs_4_0", &s->vertexShaderBuffer);
			if (!compileResult)	{
				DXTRACE_MSG("Error compiling the vertex shader!");
				return -1;
			}
			if (!createVertexShader(s->vertexShaderBuffer, &s->vertexShader)) {
				DXTRACE_MSG("Error creating the vertex shader!");
				return -1;
			}
			ID3DBlob* psBuffer = 0;
			compileResult = compileShader(descriptor.file, descriptor.pixelShader, "ps_4_0", &psBuffer);
			if (!compileResult)	{
				DXTRACE_MSG("Error compiling pixel shader!");
				return -1;
			}

			if (!createPixelShader(psBuffer, &s->pixelShader)) {
				DXTRACE_MSG("Error creating pixel shader!");
				return -1;
			}
			psBuffer->Release();
			if (descriptor.geometryShader != 0) {
				ID3DBlob* psBuffer = 0;
				compileResult = compileShader(descriptor.file, descriptor.geometryShader, "gs_4_0", &psBuffer);
				if (!compileResult)	{
					DXTRACE_MSG("Error compiling geometry shader!");
					return -1;
				}

				if (!createGeometryShader(psBuffer, &s->geometryShader)) {
					DXTRACE_MSG("Error creating geometry shader!");
					return -1;
				}
				psBuffer->Release();
			}
			s->samplerState = getSamplerState(descriptor.samplerState);
			ShaderResource* cbr = new ShaderResource(s);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SHADER);
		}

		RID ShaderParser::parse(JSONReader& reader, int childIndex) {
			ShaderDescriptor descriptor;
			descriptor.file = reader.get_string(childIndex, "file");
			descriptor.vertexShader = reader.get_string(childIndex, "vertex_shader");
			descriptor.pixelShader = reader.get_string(childIndex, "pixel_shader");
			descriptor.geometryShader = reader.get_string(childIndex, "geometry_shader");
			descriptor.model = reader.get_string(childIndex, "shader_model");
			const char* samplerStateName = reader.get_string(childIndex, "sampler_state");
			descriptor.samplerState = find(samplerStateName, ResourceType::SAMPLERSTATE);
			const char* name = reader.get_string(childIndex, "name");
			return createShader(name, descriptor);
		}
	}
}