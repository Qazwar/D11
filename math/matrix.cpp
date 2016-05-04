#include "matrix.h"
#include <math.h>
#include "vector.h"

ds::mat3 operator * (const ds::mat3& m1,const ds::mat3& m2) {
	ds::mat3 tmp;
	tmp._11 = m1._11 * m2._11 + m1._12 * m2._21 + m1._13 * m2._31;
	tmp._12 = m1._11 * m2._12 + m1._12 * m2._22 + m1._13 * m2._32;
	tmp._13 = m1._11 * m2._13 + m1._12 * m2._23 + m1._13 * m2._33;
	tmp._21 = m1._21 * m2._11 + m1._22 * m2._21 + m1._23 * m2._31;
	tmp._22 = m1._21 * m2._12 + m1._22 * m2._22 + m1._23 * m2._32;
	tmp._23 = m1._21 * m2._13 + m1._22 * m2._23 + m1._23 * m2._33;
	tmp._31 = m1._31 * m2._11 + m1._32 * m2._21 + m1._33 * m2._31;
	tmp._32 = m1._31 * m2._12 + m1._32 * m2._22 + m1._33 * m2._32;
	tmp._33 = m1._31 * m2._13 + m1._32 * m2._23 + m1._33 * m2._33;
	return tmp;
}

Vector2f operator * (const ds::mat3& m,const Vector2f& v) {
	Vector3f in(v.x,v.y,1.0f);
	Vector3f out = m * in;
	return Vector2f(out.x,out.y);
}

Vector3f operator * (const ds::mat3& m,const Vector3f& v) {
	Vector3f tmp;
	tmp.x = m._11 * v.x + m._12 * v.y + m._13 * v.z;
	tmp.y = m._21 * v.x + m._22 * v.y + m._23 * v.z;
	tmp.z = m._31 * v.x + m._32 * v.y + m._33 * v.z;
	return tmp;
}

// column mode
Vector4f operator * (const ds::mat4& m,const Vector4f& v) {
	Vector4f tmp;
	tmp.x = m._11 * v.x + m._12 * v.y + m._13 * v.z + m._14 * v.w;
	tmp.y = m._21 * v.x + m._22 * v.y + m._23 * v.z + m._24 * v.w;
	tmp.z = m._31 * v.x + m._32 * v.y + m._33 * v.z + m._34 * v.w;
	tmp.w = m._41 * v.x + m._42 * v.y + m._43 * v.z + m._44 * v.w;
	return tmp;
}

Vector3f operator * (const ds::mat4& m,const Vector3f& v) {
	Vector4f nv(v.x,v.y,v.z,1.0f);
	Vector4f tmp = m * nv;	
	return Vector3f(tmp.x,tmp.y,tmp.z);
}

// row mode
Vector4f operator * (const Vector4f& v,const ds::mat4& m) {
	Vector4f tmp;
	tmp.x = m._11 * v.x + m._21 * v.y + m._31 * v.z + m._41 * v.w;
	tmp.y = m._12 * v.x + m._22 * v.y + m._32 * v.z + m._42 * v.w;
	tmp.z = m._13 * v.x + m._23 * v.y + m._33 * v.z + m._43 * v.w;
	tmp.w = m._14 * v.x + m._24 * v.y + m._34 * v.z + m._44 * v.w;
	return tmp;
}

Vector3f operator * (const Vector3f& v,const ds::mat4& m) {
	Vector4f nv(v.x,v.y,v.z,1.0f);
	Vector4f tmp = nv * m;	
	return Vector3f(tmp.x,tmp.y,tmp.z);
}

v2 operator * (const v2& v, const ds::mat4& m) {
	Vector4f nv(v.x, v.y, 1.0f, 1.0f);
	Vector4f tmp = nv * m;
	return v2(tmp.x, tmp.y);
}


ds::mat4 operator * (const ds::mat4& m1,const ds::mat4& m2) {
	ds::mat4 tmp;
	tmp._11 = m1._11 * m2._11 + m1._12 * m2._21 + m1._13 * m2._31 + m1._14 * m2._41;
	tmp._12 = m1._11 * m2._12 + m1._12 * m2._22 + m1._13 * m2._32 + m1._14 * m2._42;
	tmp._13 = m1._11 * m2._13 + m1._12 * m2._23 + m1._13 * m2._33 + m1._14 * m2._43;
	tmp._14 = m1._11 * m2._14 + m1._12 * m2._24 + m1._13 * m2._34 + m1._14 * m2._44;

	tmp._21 = m1._21 * m2._11 + m1._22 * m2._21 + m1._23 * m2._31 + m1._24 * m2._41;
	tmp._22 = m1._21 * m2._12 + m1._22 * m2._22 + m1._23 * m2._32 + m1._24 * m2._42;
	tmp._23 = m1._21 * m2._13 + m1._22 * m2._23 + m1._23 * m2._33 + m1._24 * m2._43;
	tmp._24 = m1._21 * m2._14 + m1._22 * m2._24 + m1._23 * m2._34 + m1._24 * m2._44;

	tmp._31 = m1._31 * m2._11 + m1._32 * m2._21 + m1._33 * m2._31 + m1._34 * m2._41;
	tmp._32 = m1._31 * m2._12 + m1._32 * m2._22 + m1._33 * m2._32 + m1._34 * m2._42;
	tmp._33 = m1._31 * m2._13 + m1._32 * m2._23 + m1._33 * m2._33 + m1._34 * m2._43;
	tmp._34 = m1._31 * m2._14 + m1._32 * m2._24 + m1._33 * m2._34 + m1._34 * m2._44;

	tmp._41 = m1._41 * m2._11 + m1._42 * m2._21 + m1._43 * m2._31 + m1._44 * m2._41;
	tmp._42 = m1._41 * m2._12 + m1._42 * m2._22 + m1._43 * m2._32 + m1._44 * m2._42;
	tmp._43 = m1._41 * m2._13 + m1._42 * m2._23 + m1._43 * m2._33 + m1._44 * m2._43;
	tmp._44 = m1._41 * m2._14 + m1._42 * m2._24 + m1._43 * m2._34 + m1._44 * m2._44;

	return tmp;
}

ds::mat4 operator * (const ds::mat4& m,float v) {
	ds::mat4 tmp;
	for ( int x = 0; x < 4; ++x ) {
		for ( int y = 0; y < 4; ++y ) {
			tmp.m[x][y] = m.m[x][y] * v;
		}
	}
	return tmp;
}

namespace ds {

namespace matrix {

	mat3 identity() {
		mat3 m;
		m._11 = 1.0f;
		m._22 = 1.0f;
		m._33 = 1.0f;
		return m;
	}

	mat3 srt(float scale,float rotation,float x,float y) {		
		float ca = cos(rotation);
		float sa = sin(rotation);
		mat3 m = identity();
		m._11 = scale * ca;
		m._12 = scale * sa;
		m._13 = x;
		m._21 = scale * -sa;
		m._22 = scale * ca;
		m._23 = y;
		return m;
	}

	mat3 mat3Scale(const Vector2f& pos) {
		mat3 tm (
			pos.x,  0.0f, 0.0f,
			 0.0f, pos.y, 0.0f,
			 0.0f, 0.0f , 1.0f
			);
		return tm;
	}
	// -------------------------------------------------------
	// Translation matrix
	// -------------------------------------------------------
	mat3 mat3Transform(const Vector2f& pos) {
		/*
		mat3 tm (
			1.0f,  0.0f,  0.0f,
			0.0f,  1.0f,  0.0f,
			pos.x, pos.y, 0.0f
			);
		*/
		mat3 tm (
			1.0f, 0.0f, pos.x,
			0.0f, 1.0f, pos.y,
			0.0f, 0.0f, 1.0f
			);
		return tm;
	}

	mat3 mat3Translation(float x,float y) {
		mat3 tm (
			1.0f, 0.0f, x,
			0.0f, 1.0f, y,
			0.0f, 0.0f, 1.0f
			);
		return tm;
	}

	mat3 srt(float scaleX,float scaleY,float rotation,float x,float y) {
		mat3 sm(
			scaleX ,   0.0f, 0.0f,
			   0.0f, scaleY, 0.0f,
			   0.0f,   0.0f, 1.0f
		);
		float ca = cos(rotation);
		float sa = sin(rotation);
		mat3 rm (
			  ca,   sa, 0.0f,
			 -sa,   ca, 0.0f,
			0.0f, 0.0f, 1.0f
		);

		return sm * rm;
	}

	mat3 mat3RotationY(float angle) {
		mat3 tmp = identity();
		tmp._11 = cos(angle);
		tmp._13 = sin(angle);
		tmp._31 = -sin(angle);
		tmp._33 = cos(angle);		
		return tmp;
	}

	mat3 mat3RotationX(float angle) {
		mat3 tmp = identity();
		tmp._22 = cos(angle);
		tmp._23 = -sin(angle);
		tmp._32 = sin(angle);
		tmp._33 = cos(angle);
		return tmp;
	}

	mat3 mat3RotationZ(float angle) {
		mat3 tmp = identity();
		tmp._11 = cos(angle);
		tmp._12 = -sin(angle);
		tmp._21 = sin(angle);
		tmp._22 = cos(angle);
		return tmp;
	}

	Vector3f mul(const mat3& m,const Vector3f& v) {
		float m0 = m._11 * v.x + m._12 * v.y + m._13 * v.z;
		float m1 = m._21 * v.x + m._22 * v.y + m._23 * v.z;
		float m2 = m._31 * v.x + m._32 * v.y + m._33 * v.z;
		return Vector3f(m0,m1,m2);
	}

	Vector2f mul(const mat3& m,const Vector2f& v) {
		// we have to flip y because 0,0 is at the top left corner
		Vector3f tmp(v.x,-v.y,1.0f);
		Vector3f res = mul(m,tmp);
		return Vector2f(res.x,res.y);
	}


	mat4 m4identity() {
		mat4 m(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);		
		return m;
	}

	mat4 mat4Rotation(const Vector3f& v,float angle) {
		float L = (v.x * v.x + v.y * v.y + v.z * v.z);
		float u2 = v.x * v.x;
		float v2 = v.y * v.y;
		float w2 = v.z * v.z; 
		mat4 tmp = m4identity();
		tmp._11 = (u2 + (v2 + w2) * cos(angle)) / L;
		tmp._12 = (v.x * v.y * (1 - cos(angle)) - v.z * sqrt(L) * sin(angle)) / L;
		tmp._13 = (v.x * v.z * (1 - cos(angle)) + v.y * sqrt(L) * sin(angle)) / L;
		tmp._14 = 0.0f; 

		tmp._21 = (v.x * v.y * (1 - cos(angle)) + v.z * sqrt(L) * sin(angle)) / L;
		tmp._22 = (v2 + (u2 + w2) * cos(angle)) / L;
		tmp._23 = (v.y * v.z * (1 - cos(angle)) - v.x * sqrt(L) * sin(angle)) / L;
		tmp._24 = 0.0f; 

		tmp._31 = (v.x * v.z * (1 - cos(angle)) - v.y * sqrt(L) * sin(angle)) / L;
		tmp._32 = (v.y * v.z * (1 - cos(angle)) + v.x * sqrt(L) * sin(angle)) / L;
		tmp._33 = (w2 + (u2 + v2) * cos(angle)) / L;
		tmp._34 = 0.0f; 

		return tmp;
	} 

	mat4 mat4RotateZXY(float angleX,float angleY,float angleZ) {
		float cosX = cosf(angleX), sinX = sinf(angleX), 
			  cosY = cosf(angleY), sinY = sinf(angleY),
			  cosZ = cosf(angleZ), sinZ = sinf(angleZ);

		return mat4(
			cosY * cosZ + sinX * sinY * sinZ,   -cosX * sinZ,    sinX * cosY * sinZ - sinY * cosZ,  0,
			cosY * sinZ - sinX * sinY * cosZ,    cosX * cosZ,   -sinY * sinZ - sinX * cosY * cosZ,  0,
			cosX * sinY,                         sinX,           cosX * cosY,                       0,
			0,                                   0,              0,                                 1);
	}

	mat4 mat4RotateZXY(const Vector3f& r) {
		return mat4RotateZXY(r.x,r.y,r.z);
	}

	mat4 mat4PerspectiveFovLH(float fovy,float aspect,float zn,float zf) {
		// msdn.microsoft.com/de-de/library/windows/desktop/bb205350(v=vs.85).aspx
		float yScale = 1.0f / tan(fovy/2.0f);
		float xScale = yScale / aspect;

		mat4 tmp(
			xScale,   0.0f,           0.0f, 0.0f,
			  0.0f, yScale,           0.0f, 0.0f,
			  0.0f,   0.0f,     zf/(zf-zn), 1.0f,
			  0.0f,   0.0f, -zn*zf/(zf-zn), 0.0f
		);
		return tmp;
	}

	mat4 mat4LookAtLH(const Vector3f& eye,const Vector3f& lookAt,const Vector3f& up) {
		//mat4 tmp = m4identity();
		// see msdn.microsoft.com/de-de/library/windows/desktop/bb205342(v=vs.85).aspx
		Vector3f zAxis = normalize(lookAt - eye);
		Vector3f xAxis = normalize(cross(up,zAxis));
		Vector3f yAxis = cross(zAxis,xAxis);
		float dox = -dot(xAxis,eye);
		float doy = -dot(yAxis,eye);
		float doz = -dot(zAxis,eye);
		mat4 tmp (
		    xAxis.x, yAxis.x, zAxis.x, 0.0f,
			xAxis.y, yAxis.y, zAxis.y, 0.0f,
			xAxis.z, yAxis.z, zAxis.z, 0.0f,
			   dox,      doy,     doz, 1.0f
		);
		return tmp;
	}

	mat4 mat4OrthoLH(float w,float h,float zn,float zf) {
		// msdn.microsoft.com/de-de/library/windows/desktop/bb204940(v=vs.85).aspx
		mat4 tmp = m4identity();
		tmp._11 = 2.0f / w;
		tmp._22 = 2.0f / h;
		tmp._33 = 1.0f / (zf-zn);
		tmp._43 = zn / (zn - zf);		
		return tmp;
	}

	// -------------------------------------------------------
	// Translation matrix
	// -------------------------------------------------------

	mat4 mat4Transform(const Vector3f& pos) {
		mat4 tm (
			 1.0f,  0.0f,  0.0f, 0.0f,
			 0.0f,  1.0f,  0.0f, 0.0f,
			 0.0f,  0.0f,  1.0f, 0.0f,
			pos.x, pos.y, pos.z, 1.0f
		);
		return tm;
	}

	// -------------------------------------------------------
	// Translation matrix
	// -------------------------------------------------------

	mat4 mat4Transform(const v2& pos) {
		mat4 tm(
			1.0f, 0.0f, pos.x, 0.0f,
			0.0f, 1.0f, pos.y, 0.0f,
			0.0f, 0.0f,  1.0f, 0.0f,
			0.0f, 0.0f,  0.0f, 1.0f
			);
		return tm;
	}
	// -------------------------------------------------------
	// Scale matrix
	// -------------------------------------------------------
	mat4 mat4Scale(float scaleX,float scaleY,float scaleZ) {
		mat4 sm (
			scaleX ,   0.0f,   0.0f, 0.0f,
			   0.0f, scaleY,   0.0f, 0.0f,
			   0.0f,   0.0f, scaleZ, 0.0f,
			   0.0f,   0.0f,   0.0f, 1.0f
		);	
		return sm;
	}

	// -------------------------------------------------------
	// Scale matrix
	// -------------------------------------------------------
	mat4 mat4Scale(const Vector3f& scale) {
		mat4 sm (
			scale.x ,   0.0f,   0.0f, 0.0f,
			0.0f, scale.y,   0.0f, 0.0f,
			0.0f,   0.0f, scale.z, 0.0f,
			0.0f,   0.0f,   0.0f, 1.0f
			);	
		return sm;
	}

	// -------------------------------------------------------
	// Transpose matrix
	// -------------------------------------------------------
	mat4 mat4Transpose(const mat4& m) {
		mat4 current = m;
		mat4 tmp;
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				tmp.m[i][j] = current.m[j][i];
			}
		}
		return tmp;
	}

	// -------------------------------------------------------
	// Calculates the inverse matrix using Cramer's rule
	// -------------------------------------------------------
	mat4 mat4Inverse(const mat4& m) {
		mat4 ret;
		float tmp[12]; /* temp array for pairs */
		float src[16]; /* array of transpose source matrix */
		float det; /* determinant */
		float* dst = ret;
		float* mat = m;

		for ( int i = 0; i < 4; ++i ) {
			for ( int j = 0; j < 4; ++j ) {
				src[i*4 +j] = m.m[i][j];
			}
		}
		/* transpose matrix */
		for (int i = 0; i < 4; i++) {
			src[i] = mat[i*4];
			src[i + 4] = mat[i*4 + 1];
			src[i + 8] = mat[i*4 + 2];
			src[i + 12] = mat[i*4 + 3];
		}
		/* calculate pairs for first 8 elements (cofactors) */
		tmp[0] = src[10] * src[15];
		tmp[1] = src[11] * src[14];
		tmp[2] = src[9] * src[15];
		tmp[3] = src[11] * src[13];
		tmp[4] = src[9] * src[14];
		tmp[5] = src[10] * src[13];
		tmp[6] = src[8] * src[15];
		tmp[7] = src[11] * src[12];
		tmp[8] = src[8] * src[14];
		tmp[9] = src[10] * src[12];
		tmp[10] = src[8] * src[13];
		tmp[11] = src[9] * src[12];
		/* calculate first 8 elements (cofactors) */
		dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
		dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
		dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
		dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
		dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
		dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
		dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
		dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
		dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
		dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
		dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
		dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
		dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
		dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
		dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
		dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
		/* calculate pairs for second 8 elements (cofactors) */
		tmp[0] = src[2]*src[7];
		tmp[1] = src[3]*src[6];
		tmp[2] = src[1]*src[7];
		tmp[3] = src[3]*src[5];
		tmp[4] = src[1]*src[6];
		tmp[5] = src[2]*src[5];
		tmp[6] = src[0]*src[7];
		tmp[7] = src[3]*src[4];
		tmp[8] = src[0]*src[6];
		tmp[9] = src[2]*src[4];
		tmp[10] = src[0]*src[5];
		tmp[11] = src[1]*src[4];
		/* calculate second 8 elements (cofactors) */
		dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
		dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
		dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
		dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
		dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
		dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
		dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
		dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
		dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
		dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
		dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
		dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
		dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
		dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
		dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
		dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
		/* calculate determinant */
		det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
		/* calculate matrix inverse */
		det = 1/det;
		for (int j = 0; j < 16; j++) {
			dst[j] *= det;
		}
		for ( int i = 0; i < 4; ++i ) {
			for ( int j = 0; j < 4; ++j ) {
				ret.m[i][j] = dst[i*4 +j];
			}
		}
		return ret;
	}

	D3DXMATRIX convert(const mat4& m) {
		D3DXMATRIX tmp;
		for ( int x = 0; x < 4; ++x ) {
			for ( int y = 0; y < 4; ++y ) {
				tmp.m[x][y] = m.m[x][y];
			}
		}
		return tmp;
	}

	mat3 rotationAlign(const Vector2f& d) {
		return rotation(Vector3f(d.x,d.y,0.0f),Vector3f(1,0,0));
	}

	mat3 rotationAlign(const Vector3f& d) {
		return rotation(d,Vector3f(1,0,0));
	}

	mat3 rotation(const Vector3f& d,const Vector3f& z) {
		Vector3f dn = normalize(d);
		Vector3f axi = cross( z, dn );
		float si  = length( axi );
		float co  = dot( z, dn );
		return rotationAxisCosSin( axi/si, co, si );
	}

	mat3 rotationAxisCosSin(const Vector3f& v,float co,float si ) {
		float ic = 1.0f - co;
		return mat3( v.x*v.x*ic + co,       v.y*v.x*ic - si*v.z,    v.z*v.x*ic + si*v.y,
			v.x*v.y*ic + si*v.z,   v.y*v.y*ic + co,        v.z*v.y*ic - si*v.x,
			v.x*v.z*ic - si*v.y,   v.y*v.z*ic + si*v.x,    v.z*v.z*ic + co );
	}

	// http://www.cprogramming.com/tutorial/3d/rotationMatrices.html
	// left hand sided
	mat4 mat4RotationX(float angle) {
		mat4 sm (
			1.0f,        0.0f,       0.0f, 0.0f,
			0.0f,  cos(angle), -sin(angle), 0.0f,
			0.0f,  sin(angle), cos(angle), 0.0f,
			0.0f,        0.0f,        0.0f, 1.0f
			);	
		return sm;
	}

	mat4 mat4RotationY(float angle) {
		mat4 sm (
			 cos(angle), 0.0f, sin(angle), 0.0f,
			       0.0f, 1.0f,       0.0f, 0.0f,
			-sin(angle), 0.0f, cos(angle), 0.0f,
			       0.0f, 0.0f,       0.0f, 1.0f
			);	
		return sm;
	}
	// FIXME: wrong direction!!!!
	mat4 mat4RotationZ(float angle) {
		mat4 sm (
			 cos(angle),-sin(angle), 0.0f, 0.0f,
			 sin(angle), cos(angle), 0.0f, 0.0f,
			       0.0f,       0.0f, 1.0f, 0.0f,
			       0.0f,       0.0f, 0.0f, 1.0f
			);	
		return sm;
	}

	mat4 mat4Rotation(const Vector3f& r) {
		return mat4RotationZ(r.z) * mat4RotationY(r.y) * mat4RotationX(r.x);
	}
}

}