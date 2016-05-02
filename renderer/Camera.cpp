#include "Camera.h"

namespace ds {

	Camera::Camera() {
	}


	Camera::~Camera() {
	}

	const mat4& Camera::getViewProjectionMatrix() const {
		return _viewProjectionMatrix;
	}



	OrthoCamera::OrthoCamera(float screenWidth, float screenHeight) : Camera() {
		_viewMatrix = matrix::m4identity();
		_projectionMatrix = ds::matrix::mat4OrthoLH(screenWidth, screenHeight, 0.1f, 100.0f);
		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
	}

	OrthoCamera::~OrthoCamera() {

	}

}