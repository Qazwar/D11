#pragma once
#include "..\math\matrix.h"


namespace ds {

	class Camera {

	public:
		Camera();
		virtual ~Camera();
		const mat4& getViewProjectionMatrix() const;
		const mat4& getViewMatrix() const {
			return _viewMatrix;
		}
		const mat4& getProjectionMatrix() const {
			return _projectionMatrix;
		}
		virtual const v3& getPosition() const = 0;
		virtual const v3& getTarget() const = 0;
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
		const v3& getPosition() const {
			return _position;
		}
		const v3& getTarget() const {
			return _target;
		}
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
		void up(float unit);
		void rotateLR(bool right);
		void rotateUD(bool down);
		void setPitch(float angle);
		void resetPitch(float angle);
		void resetYaw(float angle);
		void setYAngle(float angle);
		void resetYAngle();

		void update(float elapsedTime);
		const float getAngle() const {
			return _yaw;
		}
		float getPitch() const {
			return _pitch;
		}
		const v3& getPosition() const {
			return _position;
		}
		const v3& getTarget() const {
			return _target;
		}
	private:
		void buildView();
		float _yaw;
		float _pitch;
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
		const v3& getPosition() const {
			return _position;
		}
		const v3& getTarget() const {
			return _target;
		}
	private:
		v3 _position;
		v3 _target;
	};

}
