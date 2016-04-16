#pragma once
#include <stdint.h>

typedef unsigned int IdString;

typedef uint32_t ID;

typedef uint32_t RID;

const RID INVALID_RID = UINT32_MAX;

const ID INVALID_ID = UINT32_MAX;

const float PI = 3.1415926535897932384626433832795f;
const float TWO_PI = 2.0f * PI;
const float HALF_PI = 0.5f * PI;
const float TWO_PI_INV = 1.0f / TWO_PI;

#define DEGTORAD( degree ) ((degree) * (PI / 180.0f))
#define RADTODEG( radian ) ((radian) * (180.0f / PI))

#ifndef DBG_V2
#define DBG_V2(v) "x: " << v.x << " y: " << v.y
#endif

#ifndef DBG_V3
#define DBG_V3(v) "x: " << v.x << " y: " << v.y << " z: " << v.z
#endif

#ifndef DBG_V4
#define DBG_V4(v) "x: " << v.x << " y: " << v.y << " z: " << v.z << " w: " << v.w
#endif

#ifndef DBG_RECT
#define DBG_RECT(v) "top: " << v.top << " left: " << v.left << " width: " << v.width() << " height: " << v.height()
#endif

#ifndef DBG_CLR
#define DBG_CLR(v) "r: " << v.r << " g: " << v.g << " b: " << v.b << " a: " << v.a
#endif

#ifndef DBG_TEX
#define DBG_TEX(v) "top: " << v.rect.top << " left: " << v.rect.left << " dim.x: " << v.dim.x << " dim.y: " << v.dim.y << " u1: " << v.uv.x << " v1: " << v.uv.y << " u2: " << v.uv.z << " v2: " << v.uv.w
#endif

#ifndef DBG_PNT
#define DBG_PNT(v) "x: " << v.x << " y: " << v.y
#endif

