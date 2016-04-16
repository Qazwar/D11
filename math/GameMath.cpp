#include "GameMath.h"
#include "..\utils\mtrand.h"
#include "..\utils\Log.h"

// ---------------------------------------------
//
// ---------------------------------------------

Vector2f rotate(float angle,float velocity) {
	float vx = cos(angle) * velocity;
	float vy = sin(angle) * velocity;
	return Vector2f(vx,vy);
}

namespace ds {

	namespace math {

		// http://allenchou.net/2014/02/game-math-faster-sine-cosine-with-polynomial-curves/
		float hill(float x)	{
			const float a0 = 1.0f;
			const float a2 = 2.0f / PI - 12.0f / (PI * PI);
			const float a3 = 16.0f / (PI * PI * PI) - 4.0f / (PI * PI);
			const float xx = x * x;
			const float xxx = xx * x;
			return a0 + a2 * xx + a3 * xxx;
		}

		float fastSin(float x) {
			// wrap x within [0, TWO_PI)
			const float a = x * TWO_PI_INV;
			x -= static_cast<int>(a) * TWO_PI;
			if (x < 0.0f)
				x += TWO_PI;

			// 4 pieces of hills
			if (x < HALF_PI)
				return hill(HALF_PI - x);
			else if (x < PI)
				return hill(x - HALF_PI);
			else if (x < 3.0f * HALF_PI)
				return -hill(3.0f * HALF_PI - x);
			else
				return -hill(x - 3.0f * HALF_PI);
		}

		float fastCos(float x) {
			return fastSin(x + HALF_PI);
		}

		void sincos(float x, float* s, float* c) {
			//always wrap input angle to -PI..PI
			if (x < -PI) {
				x += TWO_PI;
			}
			else {
				if (x > PI) {
					x -= TWO_PI;
				}
			}
			//compute sine
			if (x < 0) {
				*s = 1.27323954f * x + 0.405284735f * x * x;
			}
			else {
				*s = 1.27323954f * x - 0.405284735f * x * x;
			}
			//compute cosine: sin(x + PI/2) = cos(x)
			x += HALF_PI;
			if (x > PI) {
				x -= TWO_PI;
			}
			if (x < 0) {
				*c = 1.27323954f * x + 0.405284735f * x * x;
			}
			else {
				*c = 1.27323954f * x - 0.405284735f * x * x;
			}
		}

		unsigned int SIN_LOOP = 15;
		unsigned int COS_LOOP = 15;

		// sin(x) = x - x^3/3! + x^5/5! - x^7/7! + ...
		float sin_f(float x) {
			float Sum = 0;
			float Power = x;
			float Sign = 1;
			const float x2 = x * x;
			float Fact = 1.0;
			for (unsigned int i = 1; i < SIN_LOOP; i += 2) {
				Sum += Sign * Power / Fact;
				Power *= x2;
				Fact *= (i + 1) * (i + 2);
				Sign *= -1.0;
			}
			return Sum;
		}

		static const float fact3 = 0.148148148148148f;//6.75f;

		float f_sin(float a) {
			float y = 0.0f;
			float tmp = 0.0f;
			bool sign = false;
			if ((a > TWO_PI) || (a < -TWO_PI)) {
				a = fmod(a, TWO_PI);
			}
			if (a > PI) {
				a -= TWO_PI;
				sign = true;
			}

			if (a < HALF_PI) {
				y = a - a * a * a * fact3;
			}
			else {
				tmp = a - PI;
				y = -tmp + tmp * tmp * tmp * fact3;
			}

			if (sign) {
				y = -y;
			}
			return y;
		}

		// cos(x) = 1 - x^2/2! + x^4/4! - x^6/6! + ...
		float cos_f(float x) {
			float Sum = x;
			float Power = x;
			float Sign = 1.0;
			const float x2 = x * x;
			float Fact = 1.0;
			for (unsigned int i = 3; i < COS_LOOP; i += 2) {
				Power *= x2;
				Fact *= i * (i - 1);
				Sign *= -1.0;
				Sum += Sign * Power / Fact;
			}
			return Sum;
		}




		const Vector2f V2_RIGHT = Vector2f(1.0f,0.0f);

	void clamp(int* value,int min,int max) {
		if ( *value > max ) {
			*value = max;
		}
		if ( *value < min ) {
			*value = min;
		}
	}

	void clamp(float* value,float min,float max) {
		if ( *value > max ) {
			*value = max;
		}
		if ( *value < min ) {
			*value = min;
		}
	}

	float clamp(float value,float min,float max) {
		if ( value < min ) {
			return min;
		}
		if ( value > max ) {
			return max;
		}
		return value;
	}

	void getTextureCoordinates(const Rect& textureRect,int textureSize,float* u1,float* v1,float* u2,float* v2,bool useHalfTexel) {
		if ( useHalfTexel ) {
			float halfTexel = 0.5f;
			float const width   = textureSize;
			float const height  = textureSize;

			float kUOffset = halfTexel/width;
			float kVOffset = halfTexel/height;

			*u1 = static_cast<float>(textureRect.left)/width  + kUOffset;
			*v1 = static_cast<float>(textureRect.top)/height + kVOffset;  

			*u2 = *u1 + static_cast<float>(textureRect.width()) /width   - 2.0f*kUOffset;
			*v2 = *v1 + static_cast<float>(textureRect.height())/height  - 2.0f*kVOffset;
		}
		else {
			float size = static_cast<float>(textureSize);
			*u1 = static_cast<float>(textureRect.left)/size;
			*u2 = static_cast<float>(textureRect.right)/size;
			*v1 = static_cast<float>(textureRect.top)/size;
			*v2 = static_cast<float>(textureRect.bottom)/size;
		}
	}

	void getTextureCoordinates(const Rect& textureRect,int textureWidth,float textureHeight,float* u1,float* v1,float* u2,float* v2,bool useHalfTexel) {
		if ( useHalfTexel ) {
			float halfTexel = 0.5f;
			float const width   = textureWidth;
			float const height  = textureHeight;

			float kUOffset = halfTexel/width;
			float kVOffset = halfTexel/height;

			*u1 = static_cast<float>(textureRect.left)/width  + kUOffset;
			*v1 = static_cast<float>(textureRect.top)/height + kVOffset;  

			*u2 = *u1 + static_cast<float>(textureRect.width()) /width   - 2.0f*kUOffset;
			*v2 = *v1 + static_cast<float>(textureRect.height())/height  - 2.0f*kVOffset;
		}
		else {
			*u1 = static_cast<float>(textureRect.left)/textureWidth;
			*u2 = static_cast<float>(textureRect.right)/textureWidth;
			*v1 = static_cast<float>(textureRect.top)/textureHeight;
			*v2 = static_cast<float>(textureRect.bottom)/textureHeight;
		}
	}

	Vector4f getTextureCoordinates(const Rect& textureRect,float textureWidth,float textureHeight,bool useHalfTexel) {
		Vector4f ret;
		if ( useHalfTexel ) {
			float halfTexel = 0.5f;
			float const width   = textureWidth;
			float const height  = textureHeight;

			float tw = textureRect.width() / textureWidth;
			float th = textureRect.height() / textureHeight;

			float kUOffset = halfTexel/width;
			float kVOffset = halfTexel/height;

			ret.x = textureRect.left/width  + kUOffset;
			ret.y = textureRect.top/height + kVOffset;  

			ret.z = ret.x + tw   - 2.0f*kUOffset;
			ret.w = ret.y + th  - 2.0f*kVOffset;
		}
		else {
			ret.x = textureRect.left/textureWidth;
			ret.z = textureRect.right/textureWidth;
			ret.y = textureRect.top/textureHeight;
			ret.w = textureRect.bottom/textureHeight;
		}
		return ret;
	}

	Vector4f getTextureCoordinates(float top,float left,float width,float height,float textureWidth,float textureHeight,bool useHalfTexel) {
		Vector4f ret;
		float tw = width/ textureWidth;
		float th = height / textureHeight;
		if ( useHalfTexel ) {
			float halfTexel = 0.5f;

			float kUOffset = halfTexel/textureWidth;
			float kVOffset = halfTexel/textureHeight;

			ret.x = left/textureWidth  + kUOffset;
			ret.y = top/textureHeight + kVOffset;  

			ret.z = ret.x + tw   - 2.0f*kUOffset;
			ret.w = ret.y + th  - 2.0f*kVOffset;
		}
		else {
			ret.x = left/textureWidth;
			ret.z = ret.x + tw;
			ret.y = top/textureHeight;
			ret.w = ret.y + th;
		}
		return ret;
	}

	Texture buildTexture(float top, float left, float width, float height, float textureWidth, float textureHeight, bool useHalfTexel) {
		Texture ret;		
		Rect r(top, left, width, height);
		ret.rect = r;
		ret.uv = getTextureCoordinates(r, textureWidth, textureHeight, useHalfTexel);
		ret.textureID = 0;
		ret.dim = Vector2f(width ,height);
		ret.textureSize.x = textureWidth;
		ret.textureSize.y = textureHeight;
		return ret;
	}

	Texture buildTexture(const Rect& r, float textureWidth, float textureHeight, bool useHalfTexel) {
		Texture ret;
		ret.rect = r;
		ret.uv = getTextureCoordinates(r, textureWidth, textureHeight);
		ret.textureID = 0;
		ret.dim = Vector2f(r.width(), r.height());
		ret.textureSize.x = textureWidth;
		ret.textureSize.y = textureHeight;
		return ret;
	}

	void transformMousePosToWorld(Vector3f* vPickRayDir,Vector3f* vPickRayOrig) {
		// Get the Pick ray from the mouse position
		/*
		Camera *camera = gEngine->getRenderer()->getCamera();
		const D3DXMATRIX* pmatProj = &camera->getProjectionMatrix();

		Vector3f mp = Vector3f(0,0,0);
		mp.x = gEngine->getMousePosX();
		mp.y = gEngine->getMousePosY();

		// Compute the vector of the Pick ray in screen space
		D3DXVECTOR3 v;
		v.x = ( ( ( 2.0f * mp.x ) / gEngine->getWidth() ) - 1 ) / pmatProj->_11;
		v.y = -( ( ( 2.0f * mp.y ) / gEngine->getHeight() ) - 1 ) / pmatProj->_22;
		v.z = 1.0f;

		// Get the inverse view matrix
		const D3DXMATRIX matView = camera->getViewMatrix();
		const D3DXMATRIX matWorld = gEngine->getRenderer()->getWorldMatrix();
		D3DXMATRIX mWorldView = matWorld * matView;
		D3DXMATRIX m;
		D3DXMatrixInverse( &m, NULL, &mWorldView );

		// Transform the screen space Pick ray into 3D space
		vPickRayDir->x = v.x * m._11 + v.y * m._21 + v.z * m._31;
		vPickRayDir->y = v.x * m._12 + v.y * m._22 + v.z * m._32;
		vPickRayDir->z = v.x * m._13 + v.y * m._23 + v.z * m._33;

		vPickRayOrig->x = gEngine->getRenderer()->getCamera()->getPosition().x;
		vPickRayOrig->y = gEngine->getRenderer()->getCamera()->getPosition().y;
		vPickRayOrig->z = gEngine->getRenderer()->getCamera()->getPosition().z;
		*/
	}
	// ---------------------------------------------
	// Returns a smooth value between 0.0 and 1.0
	// ---------------------------------------------
	float saturate(float x) {
		if ( x < 0.0f ) {
			return 0.0f;
		}
		else if ( x > 1.0f ) {
			return 1.0f;
		}
		return x;
	}

	float smoothstep(float min,float max,float x) {
		clamp(&x,0.0f,1.0f);	
		float sp =  x * x * ( 3.0f - 2.0f * x);
		return min + sp * (max - min);
	}

	float smoothstep(float x) {
		clamp(&x,0.0f,1.0f);	
		float sp =  x * x * ( 3.0f - 2.0f * x);
		return sp;
	}

	float cubicPulse( float c, float w, float x ) {
		x = fabsf(x - c);
		if( x>w ) return 0.0f;
		x /= w;
		return 1.0f - x*x*(3.0f-2.0f*x);
	}

	float impulse( float k, float x ) {
		const float h = k*x;
		return h*expf(1.0f-h);
	}

	float almostIdentity( float x, float m, float n ) {
		if( x>m ) return x;
		const float a = 2.0f*n - m;
		const float b = 2.0f*m - 3.0f*n;
		const float t = x/m;
		return (a*t + b)*t*t + n;
	}

	// -------------------------------------------------------
	// Calculates the radial velocity. It is calculated 
	// counter clock wise
	//
	// [in] angle - angle in degrees
	// [in] velocity - the velocity
	// [out] Vector2f - a 2D vector containing the velocity
	// -------------------------------------------------------

	Vector2f getRadialVelocity(float angle,float velocity) {
		float vx = (float)cos(DEGTORAD(angle))*velocity;
		float vy = (float)sin(DEGTORAD(angle))*velocity;
		return Vector2f(vx,vy);
	}

	void radial_velocity(float angle, float velocity,v2* vel) {
		vel->x = (float)cos(angle)*velocity;
		vel->y = (float)sin(angle)*velocity;
	}

	float getAngle(float x1,float y1,float x2,float y2) {
		double x = x2 - x1;
		double y = y2 - y1;	
		double ang = atan2(y,x);
		return (float)ang;
	}

	float getAngle(const Vector2f& v1,const Vector2f& v2) {	
		if ( v1 != v2 ) {
			Vector2f vn1 = normalize(v1);
			Vector2f vn2 = normalize(v2);
			//LOG << "vn1 " << DBG_V2(vn1) << " vn2 " << DBG_V2(vn2);
			float dt = dot(vn1,vn2);		
			//LOG << "dot " << dt;
			if ( dt < -1.0f ) {
				dt = -1.0f;
			}
			if ( dt > 1.0f ) {
				dt = 1.0f;
			}
			float tmp = acos(dt);
			//LOG << "acos " << RADTODEG(tmp);
			float crs = cross(vn1,vn2);
			//LOG << "cross " << crs;
			if ( crs < 0.0f ) {
				tmp = TWO_PI - tmp;
			}
			//LOG << "new acos " << RADTODEG(tmp);
			return tmp;		
		}
		else {
			return 0.0f;
		}
	}

	float getTargetAngle(const Vector2f& target,const Vector2f& pos) {	
		Vector2f diff = target - pos;
		float angle = getAngle(V2_RIGHT,diff);
		if ( angle >= TWO_PI ) {
			angle -= TWO_PI;
		}
		return angle;
	}

	Vector2f getDistantPosition(const Vector2f& initialPosition,float angle,float radius) {
		return getDistantPosition(initialPosition.x,initialPosition.y,angle,radius);
	}

	Vector2f getDistantPosition(float x,float y,float angle,float radius) {
		Vector2f ret;
		ret.x = x + cos(angle) * radius;
		ret.y = y + sin(angle) * radius;
		return ret;
	}

	Vector2f getTargetVelocity(const Vector2f& targetPosition,const Vector2f& currentPosition,float* angle,float vel) {
		Vector2f diff = targetPosition - currentPosition;
		*angle = getAngle(diff,V2_RIGHT);
		if ( *angle < 0.0f ) {
			*angle += 2.0f * PI;
		}
		float correctedAngle = reflect(*angle);
		float vx = cos(correctedAngle) * vel;
		float vy = sin(correctedAngle) * vel;
		return Vector2f(vx,vy);
	}

	void follow(const Vector2f& targetPos,Vector2f& newPos,float* angle,float dst,float add) {
		Vector2f p = newPos;
		Vector2f diff = targetPos - p;
		//*angle = 2.0f * PI - getAngle(diff,V2_RIGHT);
		*angle = getAngle(diff,V2_RIGHT);
		if ( *angle < 0.0f ) {
			*angle += 2.0f * PI;
		}
		float dist = distance(targetPos,p);
		if ( dist > dst ) {				
			newPos += diff * add;
		}
	}

	void move_towards(const Vector2f& targetPos, Vector2f& newPos, float* angle, float velocity, float dt) {
		Vector2f p = newPos;
		Vector2f diff = targetPos - p;
		//*angle = 2.0f * PI - getAngle(diff,V2_RIGHT);
		*angle = getAngle(diff, V2_RIGHT);
		if (*angle < 0.0f) {
			*angle += 2.0f * PI;
		}
		Vector2f dn = normalize(diff);
		Vector2f v = dn * velocity;
		newPos += v * dt;		
	}

	void followRelative(const Vector2f& targetPos,Vector2f& newPos,float* angle,float dst,float percentage) {
		Vector2f p = newPos;
		Vector2f diff = targetPos - p;
		*angle = getAngle(V2_RIGHT,diff);
		if ( *angle < 0.0f ) {
			*angle += 2.0f * PI;
		}
		float dist = distance(targetPos,p);
		if ( dist > dst ) {				
			newPos.x = newPos.x + diff.x * percentage;
			newPos.y = newPos.y + diff.y * percentage;		
		}
	}

	void clipToRect(Vector2f& newPos,const Rect& rect) {
		if ( newPos.x < rect.left ) {
			newPos.x = rect.left;
		}
		if ( newPos.x > rect.right ) {
			newPos.x = rect.right;
		}
		if ( newPos.y < rect.top ) {
			newPos.y = rect.top;
		}
		if ( newPos.y > rect.bottom ) {
			newPos.y = rect.bottom;
		}
	}

	bool isOutside(Vector2f& pos,const Rect& rect) {
		bool ret = false;
		if ( pos.x > rect.right ) {
			ret = true;
		}
		if ( pos.x < rect.left ) {
			ret = true;
		}
		if ( pos.y > rect.bottom ) {
			ret = true;
		}
		if ( pos.y < rect.top ) {
			ret = true;
		}
		return ret;
	}

	bool isInside(Vector2f& pos, const Rect& rect) {
		if (pos.x > rect.right) {
			return false;
		}
		if (pos.x < rect.left) {
			return false;
		}
		if (pos.y < rect.bottom) {
			return false;
		}
		if (pos.y > rect.top) {
			return false;
		}
		return true;
	}

	bool addClamped(float *value,float add,float max,float newValue) {
		*value += add;
		if ( *value >= max ) {
			*value = newValue;
			return true;
		}
		return false;
	}

	float reflect(float angle) {
		float newAngle = 2.0f * PI - angle;
		if ( newAngle < 0.0f ) {
			newAngle += 2.0f * PI;
		}
		return newAngle;
	}

	v2 reflect(const v2& v, const v2& n) {
		//Vect2 = Vect1 - 2 * WallN * (WallN DOT Vect1)
		v2 nn = normalize(n);
		v2 ret = v - 2.0f * nn * dot(nn, v);
		return ret;
	}

	float norm(float t,float max) {
		float r = t/max;
		if ( r > 1.0f ) {
			r = 1.0f;
		}
		if ( r < 0.0f ) {
			r = 0.0f;
		}
		return r;
	}
	/*
	void interpolateColor(const Color& firstColor,const Color& secondColor,float t,Color* outColor) {
		float time = clamp(t);		
		outColor->r = firstColor.r * (1.0f - time) + secondColor.r * time;
		outColor->g = firstColor.g * (1.0f - time) + secondColor.g * time;
		outColor->b = firstColor.b * (1.0f - time) + secondColor.b * time;
		outColor->a = firstColor.a * (1.0f - time) + secondColor.a * time;
	}
	*/
	bool outside(float value,float min,float max) {
		bool ret = false;
		if ( value < min || value > max ) {
			ret = true;
		}
		return ret;
	}

		// -------------------------------------------------------
		// random
		// -------------------------------------------------------
		float random(float min,float max) {
			MTRand_open rand;
			//rand.seed(GetTickCount());
			return min + (max - min)* (float)rand();
		}

		float randomRange(float value, float variance) {
			return random(value - variance, value + variance);
		}

		// -------------------------------------------------------
		// random v2
		// -------------------------------------------------------
		v2 random(const v2& min, const v2& max) {
			float x = random(min.x, max.x);
			float y = random(min.y, max.y);
			return v2(x, y);
		}

		// -------------------------------------------------------
		// random v2 range
		// -------------------------------------------------------
		v2 randomRange(const v2& value, const v2& variance) {
			float x = randomRange(value.x, variance.x);
			float y = randomRange(value.y, variance.y);
			return v2(x, y);
		}
		// -------------------------------------------------------
		// 
		// -------------------------------------------------------
		bool checkLineCircle(const Vector2f& center, float radius,const Vector2f& lineFrom,const Vector2f& lineTo) {
			Vector2f ac = center - lineFrom;
			Vector2f ab = lineTo - lineFrom;
			float ab2 = dot(ab,ab);
			float acab = dot(ac,ab);
			float t = acab / ab2;

			if (t < 0)
				t = 0;
			else if (t > 1)
				t = 1;

			Vector2f h = ((ab * t) + lineFrom) - center;
			float h2 = dot(h,h);

			return (h2 <= (radius * radius));
		}
		// -------------------------------------------------------
		// random int
		// -------------------------------------------------------
		int random(int min,int max) {
			MTRand_open rand;
			float minf = static_cast<float>(min);
			float maxf = static_cast<float>(max) + 0.99f;
			float r = minf + (maxf - minf)* (float)rand();
			return static_cast<int>(r);
		}

		bool chanceRoll(int min) {
			int r = random(0, 100);
			return r <= min;
		}



		bool solveQuadraticFormula(const float a,const float b,const float c,float* r1,float* r2) {
			const float q = b * b - 4.0f * a * c; 
			if( q >= 0 ) {
				const float sq = sqrt(q);
				const float d = 1.0f / (2.0f * a);
				*r1 = ( -b + sq ) * d;
				*r2 = ( -b - sq ) * d;
				return true;//real roots
			}
			else {
				return false;//complex roots

			}
		}
		// -------------------------------------------------------
		// Check if two circles overlap
		// -------------------------------------------------------
		bool checkCircleIntersection(const Vector2f& p1,float r1,const Vector2f& p2,float r2) {
			Vector2f diff = p1 - p2;
			float distance = sqr_length(diff);
			float rsqr = (r1 + r2) * (r1 + r2);			
			if ( distance <= rsqr ) {
				return true;
			}
			return false;
		}

		// -------------------------------------------------------
		// Check if two circles overlap
		// -------------------------------------------------------
		bool checkCircleIntersection(const Vector2f& p1,float r1,const Vector2f& p2,float r2,float* dist,Vector2f& penVec) {
			Vector2f diff = p1 - p2;
			float distance = sqr_length(diff);
			float rsqr = (r1 + r2) * (r1 + r2);
			*dist = length(diff) - (r1 + r2);
			penVec = normalize(diff);
			penVec = penVec * (*dist);
			if ( distance <= rsqr ) {
				return true;
			}
			return false;
		}

		

		bool checkCircleBoxIntersection(const v2& circlePos, float radius, const v2& pos, const v2& dim) {
			float rx = pos.x;
			float ry = pos.y;
			float cdx = abs(circlePos.x - rx);
			float cdy = abs(circlePos.y - ry);

			if (cdx > (dim.x / 2.0f + radius)) {
				return false;
			}
			if (cdy > (dim.y / 2.0f + radius)) {
				return false;
			}

			if (cdx <= (dim.x / 2.0f)) {
				return true;
			}
			if (cdy <= (dim.y / 2.0f)) {
				return true;
			}

			float cornerDistance_sq = (cdx - dim.x / 2.0f) *  (cdx - dim.x / 2.0f) +
				(cdy - dim.y / 2.0f) * (cdy - dim.y / 2.0f);

			return (cornerDistance_sq <= (radius * radius));
		}

		bool checkCircleBoxIntersection(const v2& circlePos, float radius, const AABBox& aabBox) {
			return checkCircleBoxIntersection(circlePos, radius, aabBox.position, aabBox.extent);
		}

		bool checkCircleBoxIntersection(const v2& circlePos, float radius, const Rect& rectangle) {
			// special conversion since Rect assumes 0/0 to be top/left and we need it to be bottom/left
			Rect r = rectangle;
			r.bottom = r.top - r.height();
			float height = r.top - r.bottom;
			float rx = r.left + (r.right - r.left) * 0.5f;
			float ry = r.top - (r.top - r.bottom) * 0.5f;
			float cdx = abs(circlePos.x - rx);
			float cdy = abs(circlePos.y - ry);

			if (cdx > (r.width() / 2.0f + radius)) { 
				return false; 
			}
			if (cdy > (height / 2.0f + radius)) {
				return false; 
			}

			if (cdx <= (r.width() / 2.0f)) { 
				return true; 
			}
			if (cdy <= (height / 2.0f)) {
				return true; 
			}

			float cornerDistance_sq = (cdx - r.width() / 2.0f) *  (cdx - r.width() / 2.0f) +
				(cdy - height / 2.0f) * (cdy - height / 2.0f);

			return (cornerDistance_sq <= (radius * radius));
		}

		Vector2f getShiftVector(const Vector2f& p1,float r1,const Vector2f& p2,float r2) {
			Vector2f diff = p1 - p2;
			float maxSquareDistance = r1 + r2;
			maxSquareDistance *= maxSquareDistance;
			float squareDistance = dot(diff, diff);
			if (squareDistance > maxSquareDistance) {
				return Vector2f(0, 0);
			}
			float distance = sqrtf(squareDistance);
			if (distance > 0.0f ){
				diff.x /= distance;
				diff.y /= distance;
			}
			else{
				diff = Vector2f(1, 0);
			}

			float scaleFactor = r1 + r2 - distance;

			diff.x *= scaleFactor;
			diff.y *= scaleFactor;

			return diff;        
		}


		void clamp(Vector2f& v,float minX,float maxX,float minY,float maxY) {
			if ( v.x < minX ) {
				v.x = minX;
			}
			if ( v.x > maxX ) {
				v.x = maxX;
			}
			if ( v.y < minY ) {
				v.y = minY;
			}
			if ( v.y > maxY ) {
				v.y = maxY;
			}
		}

		bool circleSweepTest(const Vector2f& a0,const Vector2f& a1,float ra,const Vector2f& b0,const Vector2f& b1,float rb,float* u0,float* u1) {
			Vector2f va = a1 - a0;
			//vector from A0 to A1
			Vector2f vb = b1 - b0;
			//vector from B0 to B1
			Vector2f AB = b0 - a0;
			//vector from A0 to B0
			Vector2f vab = vb - va;
			//relative velocity (in normalized time)
			float rab = ra + rb;
			float a = dot(vab,vab);
			//u*u coefficient
			float b = 2.0f * dot(vab,AB);
			//u coefficient
			float c = dot(AB,AB) - rab*rab;
			//constant term
			//check if they're currently overlapping
			if( dot(AB,AB) <= rab*rab )	{
				*u0 = 0;
				*u1 = 0;
				return true;
			}
			//check if they hit each other 
			// during the frame
			if( solveQuadraticFormula( a, b, c, u0, u1 ) ) {
				if( u0 > u1 ) {
					float tmp = *u0;
					*u0 = *u1;
					*u1 = tmp;
				}
				return true;
			}
			return false;
		}

		uint32_t nextPowerOf2(uint32_t n) {
			n--;
			n |= n >> 1;
			n |= n >> 2;
			n |= n >> 4;
			n |= n >> 8;
			n |= n >> 16;
			n++;
			return n;
		}
	}

}
