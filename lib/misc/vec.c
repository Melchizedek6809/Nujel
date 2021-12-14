/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#define PI    (3.1415926535897932384626433832795f)

#include "vec.h"

#include <math.h>

vec vecCbrt(const vec a){
	return (vec){{{cbrtf(a.x),cbrtf(a.y),cbrtf(a.z)}}};
}
vec vecSqrt(const vec a){
	return (vec){{{sqrtf(a.x),sqrtf(a.y),sqrtf(a.z)}}};
}
vec vecCeil(const vec a){
	return (vec){{{ceilf(a.x),ceilf(a.y),ceilf(a.z)}}};
}
vec vecRound(const vec a){
	return (vec){{{roundf(a.x),roundf(a.y),roundf(a.z)}}};
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
	return (vec){{{ x,y,z }}};
}
vec vecNewP(const float *p){
	return (vec){{{p[0],p[1],p[2]}}};
}
vec vecNOne(){
	return (vec){{{-1.f,-1.f,-1.f}}};
}
vec vecZero(){
	return (vec){{{0.f,0.f,0.f}}};
}
vec vecOne(){
	return (vec){{{1.f,1.f,1.f}}};
}
vec vecInvert(const vec a){
	return (vec){{{-a.x,-a.y,-a.z}}};
}
vec vecAdd (const vec a, const vec b){
	return (vec){{{a.x+b.x,a.y+b.y,a.z+b.z}}};
}
vec vecAddS(const vec a, const float b){
	return (vec){{{a.x+b,a.y+b,a.z+b}}};
}
vec vecAddT(const vec a, const vec b, const vec c){
	return (vec){{{a.x+b.x+c.x,a.y+b.y+c.y,a.z+b.z+c.z}}};
}
vec vecSub (const vec a, const vec b){
	return (vec){{{a.x-b.x,a.y-b.y,a.z-b.z}}};
}
vec vecSubS(const vec a, const float b){
	return (vec){{{a.x-b,a.y-b,a.z-b}}};
}
vec vecMul (const vec a, const vec b){
	return (vec){{{a.x*b.x,a.y*b.y,a.z*b.z}}};
}
vec vecMulS(const vec a, const float b){
	return (vec){{{a.x*b,a.y*b,a.z*b}}};
}
vec vecMulT(const vec a, const vec b, const vec c){
	return (vec){{{a.x*b.x*c.x,a.y*b.y*c.y,a.z*b.z*c.z}}};
}
vec vecDiv (const vec a, const vec b){
	return (vec){{{a.x/b.x,a.y/b.y,a.z/b.z}}};
}
vec vecDivS(const vec a, const float b){
	return (vec){{{a.x/b,a.y/b,a.z/b}}};
}
vec vecMod (const vec a, const vec b){
	return (vec){{{fmodf(a.x,b.x),fmodf(a.y,b.y),fmodf(a.z,b.z)}}};
}
vec vecAbs(const vec a){
	return (vec){{{fabsf(a.x),fabsf(a.y),fabsf(a.z)}}};
}
vec vecFloor(const vec a){
	return (vec){{{floorf(a.x),floorf(a.y),floorf(a.z)}}};
}
vec vecPow(const vec a, const vec b){
	return (vec){{{powf(a.x,b.x),powf(a.y,b.y),powf(a.z,b.z)}}};
}
float vecDot (const vec a, const vec b){
	return (a.x*b.x)+(a.y*b.y)+(a.z*b.z);
}
float vecMag(const vec a){
	float dot = vecDot(a,a);
	return (dot > 0) ? sqrtf(dot) : 0;
}
float vecSum(const vec a){
	return a.x+a.y+a.z;
}
float vecAbsSum(const vec a){
	return fabsf(a.x)+fabsf(a.y)+fabsf(a.z);
}
