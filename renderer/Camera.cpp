#include "Camera.h"
#include "..\Common.h"
#include "..\utils\Log.h"

namespace ds {

	Camera::Camera() {
	}


	Camera::~Camera() {
	}

	const mat4& Camera::getViewProjectionMatrix() const {
		return _viewProjectionMatrix;
	}


	LookAtCamera::LookAtCamera(float screenWidth, float screenHeight) : Camera() {
		_projectionMatrix = matrix::mat4PerspectiveFovLH(PI * 0.25f, screenWidth / screenHeight, 0.01f, 100.0f);
		_position = v3(3, 3, -12);
		_target = v3(0, 0, 0);
		_up = v3(0, 1, 0);
		_viewMatrix = matrix::mat4LookAtLH(_position, _target, _up);
		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
	}

	LookAtCamera::~LookAtCamera() {

	}

	void LookAtCamera::setPosition(const v3& pos, const v3& target) {
		_position = pos;
		_target = target;
		_viewMatrix = matrix::mat4LookAtLH(_position, _target, _up);
		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
	}


	OrthoCamera::OrthoCamera(float screenWidth, float screenHeight) : Camera() {
		_viewMatrix = matrix::m4identity();
		_projectionMatrix = ds::matrix::mat4OrthoLH(screenWidth, screenHeight, 0.1f, 100.0f);
		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
	}

	OrthoCamera::~OrthoCamera() {

	}

	// Pitch should be in the range of [-90 ... 90] degrees and yaw
	// should be in the range of [0 ... 360] degrees.
	mat4 FPSViewRH(const v3& eye, float yaw)
	{
		// If the pitch and yaw angles are in degrees,
		// they need to be converted to radians. Here
		// I assume the values are already converted to radians.
		float cosPitch = 1.0;
		float sinPitch = 0.0f;
		float cosYaw = cos(yaw);
		float sinYaw = sin(yaw);

		v3 xaxis = { cosYaw, 0, -sinYaw };
		v3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
		v3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };

		// Create a 4x4 view matrix from the right, up, forward and eye position vectors
		mat4 viewMatrix = {
			xaxis.x, yaxis.x, zaxis.x, 0,
			xaxis.y, yaxis.y, zaxis.y, 0,
			xaxis.z, yaxis.z, zaxis.z, 0,
			-dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), 1
		};

		return viewMatrix;
	}


	FPSCamera::FPSCamera(float screenWidth, float screenHeight) : Camera() {
		_projectionMatrix = matrix::mat4PerspectiveFovLH(PI * 0.25f, screenWidth / screenHeight, 0.01f, 100.0f);
		_position = v3(3, 3, -12);
		_target = v3(0, 0, 0);
		_up = v3(0, 1, 0);
		_right = v3(1, 0, 0);
		_speed = 10.0f;
		_yaw = 0.0f;
		_viewMatrix = matrix::mat4LookAtLH(_position, _target, _up);
		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
		buildView();
	}

	FPSCamera::~FPSCamera() {

	}

	void FPSCamera::setPosition(const v3& pos, const v3& target) {
		_position = pos;
		_target = target;
		_viewMatrix = matrix::mat4LookAtLH(_position, _target, _up);
		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
		buildView();
	}

	void FPSCamera::move(float unit) {
		v3 tmp = _target * unit;
		_position = _position + tmp;
		buildView();
	}

	void FPSCamera::strafe(float unit) {
		v3 tmp = _right * unit;
		_position = _position + tmp;
		buildView();
	}

	void FPSCamera::setYAngle(float angle) {				
		_yaw += angle;
		mat3 R = matrix::mat3RotationY(_yaw);
		_right = R * _right;
		_up = R * _up;
		_target = R * _target;
		buildView();
	}

	void FPSCamera::update(float elapsedTime, const v2& mousePosition, bool buttonPressed) {
		/*
		if ( !gEngine->isConsoleActive() ) {
		*/
		if (GetAsyncKeyState('W') & 0x8000) {
			move(10.0f*elapsedTime);
		}
		if (GetAsyncKeyState('S') & 0x8000) {
			move(-10.0f*elapsedTime);
		}
		if (GetAsyncKeyState('A') & 0x8000) {
			strafe(-10.0f*elapsedTime);
		}
		if (GetAsyncKeyState('D') & 0x8000) {
			strafe(10.0f*elapsedTime);
		}
		
		if (buttonPressed) {
			// Make each pixel correspond to a quarter of a degree.
			float dx = DEGTORAD(0.25f*(mousePosition.x - _lastMousePos.x));
			float dy = DEGTORAD(0.25f*(mousePosition.y - _lastMousePos.y));
			//setPitch(dy);
			setYAngle(dx);
		}
		_lastMousePos.x = mousePosition.x;
		_lastMousePos.y = mousePosition.y;
		
	}

	void FPSCamera::buildView() {
		//_viewMatrix = matrix::mat4LookAtLH(_position, _target, _up);
		_viewMatrix = FPSViewRH(_position, _yaw);
		/*
		// Keep camera's axes orthogonal to each other and of unit length.
		v3 L = normalize(_target);
		v3 U = cross(_target, _right);
		U = normalize(U);
		v3 R = cross(U, L);
		R = normalize(R);

		// Fill in the view matrix entries.
		_target = L;
		_right = R;
		_up = U;

		float x = -dot(_position, R);
		float y = -dot(_position, U);
		float z = -dot(_position, L);

		_viewMatrix._11 = _right.x;
		_viewMatrix._21 = _right.y;
		_viewMatrix._31 = _right.z;
		_viewMatrix._41 = x;

		_viewMatrix._12 = _up.x;
		_viewMatrix._22 = _up.y;
		_viewMatrix._32 = _up.z;
		_viewMatrix._42 = y;

		_viewMatrix._13 = _target.x;
		_viewMatrix._23 = _target.y;
		_viewMatrix._33 = _target.z;
		_viewMatrix._43 = z;

		_viewMatrix._14 = 0.0f;
		_viewMatrix._24 = 0.0f;
		_viewMatrix._34 = 0.0f;
		_viewMatrix._44 = 1.0f;
		*/
		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
	}
}