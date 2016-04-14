#pragma once
#include "math_types.h"
#include <d3dx9math.h>

ds::mat3 operator * (const ds::mat3& m1,const ds::mat3& m2);

Vector3f operator * (const ds::mat3& m,const Vector3f& v);

Vector2f operator * (const ds::mat3& m,const Vector2f& v);

Vector4f operator * (const ds::mat4& m,const Vector4f& v);

Vector4f operator * (const Vector4f& v,const ds::mat4& m);

Vector3f operator * (const ds::mat4& m,const Vector3f& v);

Vector3f operator * (const Vector3f& v,const ds::mat4& m);

v2 operator * (const v2& v, const ds::mat4& m);

ds::mat4 operator * (const ds::mat4& m1,const ds::mat4& m2);

ds::mat4 operator * (const ds::mat4& m,float v);

namespace ds {

	namespace matrix {

		mat3 identity();

		mat3 srt(float scale,float rotation,float x,float y);

		mat3 srt(float scaleX,float scaleY,float rotation,float x,float y);

		Vector3f mul(const mat3& m,const Vector3f& v);

		Vector2f mul(const mat3& m,const Vector2f& v);

		mat4 m4identity();

		mat3 mat3Transform(const Vector2f& pos);

		mat3 mat3Translation(float x,float y);

		mat3 mat3Scale(const Vector2f& pos);

		mat3 mat3RotationY(float angle);

		mat3 mat3RotationX(float angle);

		mat3 mat3RotationZ(float angle);

		mat4 mat4Scale(float scaleX,float scaleY,float scaleZ);

		mat4 mat4Scale(const Vector3f& scale);

		mat4 mat4Transform(const Vector3f& pos);

		mat4 mat4Transform(const v2& pos);

		mat4 mat4Rotation(const Vector3f& v,float angle);

		mat4 mat4Rotation(const Vector3f& r);

		mat4 mat4RotationX(float angle);

		mat4 mat4RotationY(float angle);

		mat4 mat4RotationZ(float angle);

		mat4 mat4RotateZXY(float angleX,float angleY,float angleZ);

		mat4 mat4RotateZXY(const Vector3f& r);

		mat4 mat4PerspectiveFovLH(float fovy,float aspect,float zn,float zf);

		mat4 mat4LookAtLH(const Vector3f& eye,const Vector3f& lookAt,const Vector3f& up);	
				
		mat4 mat4OrthoLH(float w,float h,float zn,float zf);

		mat4 mat4Transpose(const mat4& m);

		mat4 mat4Inverse(const mat4& m);

		D3DXMATRIX convert(const mat4& m);

		mat3 rotationAlign(const Vector2f& d);

		mat3 rotationAlign(const Vector3f& d);

		mat3 rotation(const Vector3f& d,const Vector3f& z);

		mat3 rotationAxisCosSin(const Vector3f& v,float co,float si );
	}

}
