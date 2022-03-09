/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#define PI    (3.1415926535897932384626433832795)

#include "vec.h"
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
	vec ret;
	ret.x = cbrtf(a.x);
	ret.y = cbrtf(a.y);
	ret.z = cbrtf(a.z);
	return ret;
}
vec vecSqrt(const vec a){
	vec ret;
	ret.x = sqrtf(a.x);
	ret.y = sqrtf(a.y);
	ret.z = sqrtf(a.z);
	return ret;
}
vec vecCeil(const vec a){
	vec ret;
	ret.x = ceilf(a.x);
	ret.y = ceilf(a.y);
	ret.z = ceilf(a.z);
	return ret;
}
vec vecRound(const vec a){
	vec ret;
	ret.x = roundf(a.x);
	ret.y = roundf(a.y);
	ret.z = roundf(a.z);
	return ret;
}

vec vecNorm(const vec a){
	vec ret = a;
	float mag = vecMag(a);
	if (mag > 0) {
		float invLen = 1 / mag;
		ret.x *= invLen;
		ret.y *= invLen;
		ret.z *= invLen;
	}
	return ret;
}

vec vecCross(const vec a, const vec b){
	vec ret;
	ret.x =   a.y * b.z - a.z * b.y;
	ret.y = -(a.x * b.z - a.z * b.x);
	ret.z =   a.x * b.y - a.y * b.x;
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
	return ret;
}

vec vecDegToVec(const vec a){
	vec ret;
	ret.x = cosf((a.x-90.f)*PI180) * cosf((-a.y)*PI180);
	ret.y = sinf((-a.y)*PI180);
	ret.z = sinf((a.x-90.f)*PI180) * cosf((-a.y)*PI180);
	return ret;
}

vec vecNew (float x, float y, float z){
	vec ret;
	ret.x = x;
	ret.y = y;
	ret.z = z;
	return ret;
}
vec vecNewP(const float *p){
	vec ret;
	ret.x = p[0];
	ret.y = p[1];
	ret.z = p[2];
	return ret;
}
vec vecNOne(){
	vec ret;
	ret.x = -1.f;
	ret.y = -1.f;
	ret.z = -1.f;
	return ret;
}
vec vecZero(){
	vec ret;
	ret.x = 0.f;
	ret.y = 0.f;
	ret.z = 0.f;
	return ret;
}
vec vecOne(){
	vec ret;
	ret.x = 1.f;
	ret.y = 1.f;
	ret.z = 1.f;
	return ret;
}
vec vecInvert(const vec a){
	vec ret;
	ret.x = -a.x;
	ret.y = -a.y;
	ret.z = -a.z;
	return ret;
}
vec vecAdd (const vec a, const vec b){
	vec ret;
	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	ret.z = a.z + b.z;
	return ret;
}
vec vecAddS(const vec a, const float b){
	vec ret;
	ret.x = a.x + b;
	ret.y = a.y + b;
	ret.z = a.z + b;
	return ret;
}
vec vecAddT(const vec a, const vec b, const vec c){
	vec ret;
	ret.x = a.x + b.x + c.x;
	ret.y = a.y + b.y + c.y;
	ret.z = a.z + b.z + c.z;
	return ret;
}
vec vecSub (const vec a, const vec b){
	vec ret;
	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	ret.z = a.z - b.z;
	return ret;
}
vec vecSubS(const vec a, const float b){
	vec ret;
	ret.x = a.x - b;
	ret.y = a.y - b;
	ret.z = a.z - b;
	return ret;
}
vec vecMul (const vec a, const vec b){
	vec ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	ret.z = a.z * b.z;
	return ret;
}
vec vecMulS(const vec a, const float b){
	vec ret;
	ret.x = a.x * b;
	ret.y = a.y * b;
	ret.z = a.z * b;
	return ret;
}
vec vecMulT(const vec a, const vec b, const vec c){
	vec ret;
	ret.x = a.x * b.x * c.x;
	ret.y = a.y * b.y * c.y;
	ret.z = a.z * b.z * c.z;
	return ret;
}
vec vecDiv (const vec a, const vec b){
	vec ret;
	ret.x = a.x / b.x;
	ret.y = a.y / b.y;
	ret.z = a.z / b.z;
	return ret;
}
vec vecDivS(const vec a, const float b){
	vec ret;
	ret.x = a.x / b;
	ret.y = a.y / b;
	ret.z = a.z / b;
	return ret;
}
vec vecMod (const vec a, const vec b){
	vec ret;
	ret.x = fmodf(a.x, b.x);
	ret.y = fmodf(a.y, b.y);
	ret.z = fmodf(a.z, b.z);
	return ret;
}
vec vecAbs(const vec a){
	vec ret;
	ret.x = fabsf(a.x);
	ret.y = fabsf(a.y);
	ret.z = fabsf(a.z);
	return ret;
}
vec vecFloor(const vec a){
	vec ret;
	ret.x = floorf(a.x);
	ret.y = floorf(a.y);
	ret.z = floorf(a.z);
	return ret;
}
vec vecPow(const vec a, const vec b){
	vec ret;
	ret.x = powf(a.x, b.x);
	ret.y = powf(a.y, b.y);
	ret.z = powf(a.z, b.z);
	return ret;
}
float vecDot (const vec a, const vec b){
	return (a.x*b.x)+(a.y*b.y)+(a.z*b.z);
}
float vecMag(const vec a){
	float dot = vecDot(a,a);
	return (dot > 0) ? sqrtf(dot) : 0;
}
float vecSum(const vec a){
	return a.x + a.y + a.z;
}
float vecAbsSum(const vec a){
	return fabsf(a.x) + fabsf(a.y) + fabsf(a.z);
}
vec vecReflect(const vec i, const vec n){
	return vecMul(vecMulS(vecSubS(i,2.f), vecDot(n,i)),n);
}
