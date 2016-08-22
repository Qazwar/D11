#include "GameMath.h"
#include "..\utils\Log.h"
#include "math.h"
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

	const Vector2f V2_RIGHT = Vector2f(1.0f,0.0f);
	
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
	/*
	float smoothstep(float min,float max,float x) {
		x = math::clamp(x,0.0f,1.0f);	
		float sp =  x * x * ( 3.0f - 2.0f * x);
		return min + sp * (max - min);
	}

	float smoothstep(float x) {
		x = math::clamp(x, 0.0f, 1.0f);
		float sp =  x * x * ( 3.0f - 2.0f * x);
		return sp;
	}
	*/
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
			v2 p;
			p.x = aabBox.position.x;
			p.y = aabBox.position.y;
			v2 e;
			e.x = aabBox.extent.x;
			e.y = aabBox.extent.y;
			return checkCircleBoxIntersection(circlePos, radius, p, e);
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
