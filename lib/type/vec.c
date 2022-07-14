/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
#endif

#include <math.h>

#ifdef __WATCOMC__
#define fabsf(X) fabs(X)
#define floorf(X) floor(X)
#define powf(X,Y) pow(X,Y)
#define fmodf(X,Y) fmod(X,Y)
#define sqrtf(X) sqrt(X)
#define cbrtf(X) cbrt(X)
#define ceilf(X) ceil(X)
#define roundf(X) round(X)
#define sinf(X) sin(X)
#define cosf(X) cos(X)
#define atan2f(X,Y) atan2(X,Y)
#endif

vec vecCbrt(const vec a){
	return (vec){cbrtf(a.x), cbrtf(a.y), cbrtf(a.z), cbrtf(a.w)};
}
vec vecSqrt(const vec a){
	return (vec){sqrtf(a.x), sqrtf(a.y), sqrtf(a.z), sqrtf(a.w)};
}
vec vecCeil(const vec a){
	return (vec){ceilf(a.x), ceilf(a.y), ceilf(a.z), ceilf(a.z)};
}
vec vecRound(const vec a){
	return (vec){roundf(a.x), roundf(a.y), roundf(a.z), roundf(a.z)};
}

vec vecNorm(const vec a){
	vec ret = a;
	float mag = vecMag(a);
	if (mag > 0) {
		float invLen = 1 / mag;
		ret.x *= invLen;
		ret.y *= invLen;
		ret.z *= invLen;
		ret.w *= invLen;
	}
	return ret;
}

vec vecCross(const vec a, const vec b){
	vec ret;
	ret.x =   a.y * b.z - a.z * b.y;
	ret.y = -(a.x * b.z - a.z * b.x);
	ret.z =   a.x * b.y - a.y * b.x;
	ret.w = 1.f;
	return ret;
}

vec vecRotate(const vec a, const vec b, const float rad){
	float cos_theta = cosf(rad);
	float sin_theta = sinf(rad);
	vec ret = vecMulS(vecMulS(b,vecDot(a,b)),1-cos_theta);
	ret = vecAdd(ret,vecMulS(vecCross(b,a),sin_theta));
	ret = vecAdd(ret,vecMulS(a,cos_theta));
	return ret;
}

vec vecVecToDeg(const vec a){
	vec ret;
	ret.x = atan2f( a.z,a.x)*180/PI + 90.f;
	ret.y = atan2f(-a.y,a.z)*180/PI;
	ret.z = 0.f;
	ret.w = 0.f;
	return ret;
}

vec vecDegToVec(const vec a){
	vec ret;
	ret.x = cosf((a.x-90.f)*PI/180) * cosf((-a.y)*PI/180);
	ret.y = sinf((-a.y)*PI/180);
	ret.z = sinf((a.x-90.f)*PI/180) * cosf((-a.y)*PI/180);
	ret.w = 0.f;
	return ret;
}

vec vecNew (float x, float y, float z, float w){
	return (vec){x,y,z,w};
}
vec vecNewP(const float *p){
	return (vec){p[0], p[1], p[2], p[3]};
}
vec vecNOne(){
	return (vec){-1.f, -1.f, -1.f, -1.f};
}
vec vecZero(){
	return (vec){0.f, 0.f, 0.f, 0.f};
}
vec vecOne(){
	return (vec){1.f, 1.f, 1.f, 1.f};
}
vec vecInvert(const vec a){
	return (vec){-a.x,-a.y,-a.z, -a.w};
}
vec vecAdd (const vec a, const vec b){
	return (vec){a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w};
}
vec vecAddS(const vec a, const float b){
	return (vec){a.x+b, a.y+b, a.z+b, a.w+b};
}
vec vecSub (const vec a, const vec b){
	return (vec){a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w};
}
vec vecSubS(const vec a, const float b){
	return (vec){a.x-b, a.y-b, a.z-b, a.w-b};
}
vec vecMul (const vec a, const vec b){
	return (vec){a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w};
}
vec vecMulS(const vec a, const float b){
	return (vec){a.x*b, a.y*b, a.z*b, a.w*b};
}
vec vecDiv (const vec a, const vec b){
	return (vec){a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w};
}
vec vecDivS(const vec a, const float b){
	return (vec){a.x/b, a.y/b, a.z/b, a.w/b};
}
vec vecMod (const vec a, const vec b){
	return (vec){fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z), fmodf(a.w, b.w)};
}
vec vecAbs(const vec a){
	return (vec){fabsf(a.x), fabsf(a.y), fabsf(a.z), fabsf(a.w)};
}
vec vecFloor(const vec a){
	return (vec){floorf(a.x), floorf(a.y), floorf(a.z), floorf(a.w)};
}
vec vecPow(const vec a, const vec b){
	return (vec){powf(a.x, b.x), powf(a.y, b.y), powf(a.z, b.z), powf(a.w, b.w)};
}
float vecDot (const vec a, const vec b){
	return (a.x*b.x)+(a.y*b.y)+(a.z*b.z)+(a.w*b.w);
}
float vecMag(const vec a){
	float dot = vecDot(a,a);
	return (dot > 0) ? sqrtf(dot) : 0;
}
float vecSum(const vec a){
	return a.x + a.y + a.z + a.w;
}
float vecAbsSum(const vec a){
	return fabsf(a.x) + fabsf(a.y) + fabsf(a.z) + fabsf(a.w);
}
vec vecReflect(const vec i, const vec n){
	return vecMul(vecMulS(vecSubS(i,2.f), vecDot(n,i)),n);
}
