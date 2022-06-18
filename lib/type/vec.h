#ifndef NUJEL_LIB_MISC_VEC
#define NUJEL_LIB_MISC_VEC
#include "../common.h"

vec   vecNew      (float x, float y, float z, float w);
vec   vecNewP     (const float *p);
vec   vecNOne     ();
vec   vecZero     ();
vec   vecOne      ();
vec   vecInvert   (const vec a);
vec   vecAdd      (const vec a, const vec   b);
vec   vecAddS     (const vec a, const float b);
vec   vecSub      (const vec a, const vec   b);
vec   vecSubS     (const vec a, const float b);
vec   vecMul      (const vec a, const vec   b);
vec   vecMulS     (const vec a, const float b);
vec   vecDiv      (const vec a, const vec   b);
vec   vecDivS     (const vec a, const float b);
vec   vecMod      (const vec a, const vec   b);
vec   vecAbs      (const vec a);
vec   vecFloor    (const vec a);
vec   vecPow      (const vec a, const vec b);
float vecDot      (const vec a, const vec b);
float vecMag      (const vec a);
float vecSum      (const vec a);
float vecAbsSum   (const vec a);
vec   vecSqrt     (const vec a);
vec   vecCross    (const vec a, const vec b);
vec   vecRotate   (const vec a, const vec b, const float rad);
vec   vecNorm     (const vec a);
vec   vecVecToDeg (const vec a);
vec   vecDegToVec (const vec a);
vec   vecCeil     (const vec a);
vec   vecRound    (const vec a);
vec   vecReflect  (const vec i, const vec n);
vec   vecCbrt     (const vec a);

#endif
