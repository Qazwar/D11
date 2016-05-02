#pragma once
#include "..\math\matrix.h"


namespace ds {

	class Camera {

	public:
		Camera();
		virtual ~Camera();
		const mat4& getViewProjectionMatrix() const;
	protected:
		mat4 _viewMatrix;
		mat4 _projectionMatrix;
		mat4 _viewProjectionMatrix;
	};

	class OrthoCamera : public Camera {

	public:
		OrthoCamera(float screenWidth, float screenHeight);
		virtual ~OrthoCamera();
	};

}
