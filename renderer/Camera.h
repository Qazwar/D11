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

	class LookAtCamera : public Camera {

	public:
		LookAtCamera(float screenWidth, float screenHeight);				
		virtual ~LookAtCamera();
		void setPosition(const v3& pos, const v3& lookAt);		
	private:
		v3 _position;
		v3 _target;
		v3 _up;
		
	};

	class FPSCamera : public Camera {

	public:
		FPSCamera(float screenWidth, float screenHeight);
		virtual ~FPSCamera();
		void setPosition(const v3& pos, const v3& lookAt);
		void setSpeed(float spd) {
			_speed = spd;
		}
		// moving the camera
		void move(float unit);
		void strafe(float unit);
		void rotateLR(bool right);
		void rotateUD(bool down);
		void setPitch(float angle);
		void setYAngle(float angle);
		void resetYAngle();

		void update(float elapsedTime, const v2& mousePosition);
		const float getAngle() const {
			return _yaw;
		}
		const v3& getPosition() const {
			return _position;
		}
	private:
		void buildView();
		float _yaw;
		v3 _position;
		v3 _target;
		v3 _up;
		v3 _right;
		float _speed;
		v2 _lastMousePos;
	};

	class OrthoCamera : public Camera {

	public:
		OrthoCamera(float screenWidth, float screenHeight);
		virtual ~OrthoCamera();
	};

}
