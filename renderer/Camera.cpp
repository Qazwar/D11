#include "Camera.h"
#include "..\Common.h"
#include "..\utils\Log.h"
#include "..\base\InputSystem.h"
#include <d3dx9math.h>

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
		_projectionMatrix = ds::matrix::mat4OrthoLH(screenWidth, screenHeight, 0.1f, 1.0f);
		//D3DXMATRIX m_orthoMatrix;
		//D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, 0.1f, 100.0f);
		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
	}

	OrthoCamera::~OrthoCamera() {

	}

	// Pitch should be in the range of [-90 ... 90] degrees and yaw
	// should be in the range of [0 ... 360] degrees.
	mat4 FPSViewRH(const v3& eye, float pitch, float yaw)
	{
		// If the pitch and yaw angles are in degrees,
		// they need to be converted to radians. Here
		// I assume the values are already converted to radians.
		float cosPitch = cos(pitch);
		float sinPitch = sin(pitch);
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
		_pitch = 0.0f;
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
		mat3 R = matrix::mat3RotationX(_pitch);
		tmp = R * tmp;
		_position = _position + tmp;
		buildView();
	}

	void FPSCamera::strafe(float unit) {
		v3 tmp = _right * unit;
		_position = _position + tmp;
		buildView();
	}

	void FPSCamera::up(float unit) {
		v3 tmp = _up * unit;
		_position = _position + tmp;
		buildView();
	}

	void FPSCamera::setYAngle(float angle) {				
		_yaw += angle;
		mat3 R = matrix::mat3RotationY(_yaw);
		_right = R * v3(1,0,0);
		_target = R * v3(0,0,1);
		buildView();
	}

	void FPSCamera::resetYaw(float angle) {
		_yaw = angle;
		mat3 R = matrix::mat3RotationY(_yaw);
		_right = R * v3(1, 0, 0);
		_target = R * v3(0, 0, 1);
		buildView();
	}

	void FPSCamera::setPitch(float angle) {
		_pitch -= angle;
		mat3 R = matrix::mat3RotationX(_pitch);
		_right = R * v3(1, 0, 0);
		_target = R * v3(0, 0, 1);
		buildView();
	}

	void FPSCamera::resetPitch(float angle) {
		_pitch = angle;
		mat3 R = matrix::mat3RotationX(_pitch);
		_right = R * v3(1, 0, 0);
		_target = R * v3(0, 0, 1);
		buildView();
	}

	void FPSCamera::resetYAngle() {
		_yaw = 0.0f;
		mat3 R = matrix::mat3RotationY(_yaw);
		_right = R * v3(1, 0, 0);
		_target = R * v3(0, 0, 1);
		buildView();
	}

	void FPSCamera::update(float elapsedTime) {
		if (input::getKeyState('W')) {
			move(5.0f*elapsedTime);
		}
		if (input::getKeyState('S')) {
			move(-5.0f*elapsedTime);
		}
		if (input::getKeyState('A')) {
			strafe(-5.0f*elapsedTime);
		}
		if (input::getKeyState('D')) {
			strafe(5.0f*elapsedTime);
		}
		if (input::getKeyState('Q')) {
			up(5.0f*elapsedTime);
		}
		if (input::getKeyState('E')) {
			up(-5.0f*elapsedTime);
		}
		v2 mp = input::getMousePosition();
		if (input::isMouseButtonPressed(0)) {
			
			// Make each pixel correspond to a quarter of a degree.
			float dx = DEGTORAD(0.25f*(mp.x - _lastMousePos.x));
			float dy = DEGTORAD(0.25f*(mp.y - _lastMousePos.y));
			setPitch(dy);
			setYAngle(dx);
		}
		_lastMousePos.x = mp.x;
		_lastMousePos.y = mp.y;
		
	}

	void FPSCamera::buildView() {
		_viewMatrix = FPSViewRH(_position, _pitch, _yaw);
		_viewProjectionMatrix = _viewMatrix * _projectionMatrix;
	}
}