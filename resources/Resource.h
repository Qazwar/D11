#pragma once

namespace ds {

	class BaseResource {

	public:
		BaseResource() {}
		virtual ~BaseResource() {}

	};

	template<class T>
	class AbstractResource : public BaseResource {

	public:
		AbstractResource(T t) : _data(t) {}
		virtual ~AbstractResource() {}
		T get() {
			return _data;
		}
		const T get() const {
			return _data;
		}
	protected:
		T _data;
	};

	class ConstantBufferResource : public AbstractResource<ID3D11Buffer*> {

	public:
		ConstantBufferResource(ID3D11Buffer* t) : AbstractResource(t) {}
		virtual ~ConstantBufferResource() {
			if (_data != 0) {
				_data->Release();
				_data = 0;
			}
		}
	};

	class InputLayoutResource : public AbstractResource<ID3D11InputLayout*> {

	public:
		InputLayoutResource(ID3D11InputLayout* t,int size) : AbstractResource(t) , _size(size) {}
		virtual ~InputLayoutResource() {
			if (_data != 0) {
				_data->Release();
				_data = 0;
			}
		}
		int size() const {
			return _size;
		}
	private:
		int _size;
	};

	class VertexBufferResource : public AbstractResource<ID3D11Buffer*> {

	public:
		VertexBufferResource(ID3D11Buffer* t,int size) : AbstractResource(t) , _size(size) {}
		virtual ~VertexBufferResource() {
			if (_data != 0) {
				_data->Release();
				_data = 0;
			}
		}
		int size() const {
			return _size;
		}
	private:
		int _size;
	};

	class BitmapfontResource : public AbstractResource<Bitmapfont*> {

	public:
		BitmapfontResource(Bitmapfont* t) : AbstractResource(t) {}
		virtual ~BitmapfontResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class IndexBufferResource : public AbstractResource<ID3D11Buffer*> {

	public:
		IndexBufferResource(ID3D11Buffer* t) : AbstractResource(t) {}
		virtual ~IndexBufferResource() {
			if (_data != 0) {
				_data->Release();
				_data = 0;
			}
		}
	};

	class ShaderResourceViewResource : public AbstractResource<ID3D11ShaderResourceView*> {

	public:
		ShaderResourceViewResource(ID3D11ShaderResourceView* t) : AbstractResource(t) {}
		virtual ~ShaderResourceViewResource() {
			if (_data != 0) {
				_data->Release();
				_data = 0;
			}
		}
	};

	class BlendStateResource : public AbstractResource<ID3D11BlendState*> {

	public:
		BlendStateResource(ID3D11BlendState* t) : AbstractResource(t) {}
		virtual ~BlendStateResource() {
			if (_data != 0) {
				_data->Release();
				_data = 0;
			}
		}
	};

	class SpriteBufferResource : public AbstractResource<SpriteBuffer*> {

	public:
		SpriteBufferResource(SpriteBuffer* t) : AbstractResource(t) {}
		virtual ~SpriteBufferResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class QuadBufferResource : public AbstractResource<QuadBuffer*> {

	public:
		QuadBufferResource(QuadBuffer* t) : AbstractResource(t) {}
		virtual ~QuadBufferResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class MeshResource : public AbstractResource<Mesh*> {

	public:
		MeshResource(Mesh* t) : AbstractResource(t) {}
		virtual ~MeshResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class MeshBufferResource : public AbstractResource<MeshBuffer*> {

	public:
		MeshBufferResource(MeshBuffer* t) : AbstractResource(t) {}
		virtual ~MeshBufferResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class SamplerStateResource : public AbstractResource<ID3D11SamplerState*> {

	public:
		SamplerStateResource(ID3D11SamplerState* t) : AbstractResource(t) {}
		virtual ~SamplerStateResource() {
			if (_data != 0) {
				_data->Release();
				_data = 0;
			}
		}
	};

	class GUIDialogResource : public AbstractResource<GUIDialog*> {

	public:
		GUIDialogResource(GUIDialog* t) : AbstractResource(t) {}
		virtual ~GUIDialogResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class WorldResource : public AbstractResource<World*> {

	public:
		WorldResource(World* t) : AbstractResource(t) {}
		virtual ~WorldResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class Scene;

	class SceneResource : public AbstractResource<Scene*> {

	public:
		SceneResource(Scene* t) : AbstractResource(t) {}
		virtual ~SceneResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class Camera;

	class CameraResource : public AbstractResource<Camera*> {

	public:
		CameraResource(Camera* t) : AbstractResource(t) {}
		virtual ~CameraResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class RenderTarget;

	class RenderTargetResource : public AbstractResource<RenderTarget*> {

	public:
		RenderTargetResource(RenderTarget* t) : AbstractResource(t) {}
		virtual ~RenderTargetResource() {
			if (_data != 0) {
				delete _data;
			}
		}
	};

	class ShaderResource : public AbstractResource<Shader*> {

	public:
		ShaderResource(Shader* t) : AbstractResource(t) {}
		virtual ~ShaderResource() {
			if (_data != 0) {
				if (_data->vertexShader != 0) {
					_data->vertexShader->Release();
					_data->vertexShader = 0;
				}
				if (_data->pixelShader != 0) {
					_data->pixelShader->Release();
					_data->pixelShader = 0;
				}
				if (_data->geometryShader != 0) {
					_data->geometryShader->Release();
					_data->geometryShader = 0;
				}
				if (_data->vertexShaderBuffer != 0) {
					_data->vertexShaderBuffer->Release();
					_data->vertexShaderBuffer = 0;
				}
				delete _data;
			}
		}
	};

}