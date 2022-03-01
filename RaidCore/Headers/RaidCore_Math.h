/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */
#ifndef __RC_X_MATH__H_

#include "RaidCore_Common.h"
//-----------------
#include <float.h>
#include <math.h>

namespace game_math {
    // DECLARE
    static const int32 failureValue = ~0;
    static const real32 epsilon = 0.00001f; // float number error margin. ex. (A +- epsilon) == A
    static const real32 floatOne = 0.999999f; // any number that is within this is considered equal. (A * floatOne) == A
    static const real32 angle2Radians = 0.01745329f;
    static const real32 minOffset = 0.0001f; // coordinate minimal offset
}

namespace mathf {
    // NOTE(Roman): adding the basic float math functions; abs, ceil, floor, round, and arc-cosine
    __inline real32  __CRTDECL absf(_In_ real32 _X) {
        // super fast abs
        real32 result = (real32(_X >= 0.0f) - real32(_X < 0.0f)) * _X;
        return (result);
    }

    __inline real32  __CRTDECL ceilf(_In_ real32 _X) {
        //
        // NOTE(Roman): any value of _X that is >= (_X + epsilon) is floored to floor(_X + 1.0)
        //              otherwise we get floor(_X) as return value
        return static_cast<real32>(static_cast<int32>(_X + game_math::floatOne));
    }
    __inline real32  __CRTDECL floorf(_In_ real32 _X) {
        return static_cast<real32>(static_cast<int32>(_X));
    }
    __inline real32  __CRTDECL roundf(_In_ real32 _X) {
        return static_cast<real32>(static_cast<int32>(_X + 0.5f));
    }
    __inline real32  __CRTDECL acosf(_In_ real32 _X) {
        //return ((-0.6981317f * _X * _X - 0.87266463f) * _X + 1.57079633f);
        real32 negate = real32(_X < 0);
        _X = absf(_X);
        real32 ret = -0.0187293f;
        ret = ret * _X;
        ret = ret + 0.0742610f;
        ret = ret * _X;
        ret = ret - 0.2121144f;
        ret = ret * _X;
        ret = ret + 1.5707288f;
        ret = ret * sqrtf(1.0f - _X);
        ret = ret - 2.0f * negate * ret;
        return negate * 3.14159265358979f + ret;
    }
}


//
// Note(Roman): int based vectors
inline vec2i v2i(int32 x, int32 y) {
    vec2i pt = { x, y };
    return(pt);
}
inline bool32 operator ==(vec2i& l, vec2i& r) {
    return ((l.x == r.x) && (l.y == r.y)) ? true32 : false32;
}

inline vec3i v3i(int32 x, int32 y, int32 z) {
    vec3i pt = { x, y, z };
    return(pt);
}
inline vec3i operator >>(const vec3i& v, int32 i) {
    vec3i result = {
        v.x >> i,
        v.y >> i,
        v.z >> i
    };
    return (result);
}
inline bool32 operator ==(vec3i& l, vec3i& r) {
    return ((l.x == r.x) && (l.y == r.y) && (l.z == r.z)) ? true32 : false32;
}

inline vec4i v4i(int32 x, int32 y, int32 z, int32 w = 1) {
    vec4i pt = { x, y, z, w };
    return pt;
}
inline bool32 operator ==(vec4i& l, vec4i& r) {
    return ((l.x == r.x) && (l.y == r.y) && (l.z == r.z) && (l.w == r.w)) ? true32 : false32;
}

//
// Note(Roman): float based vectors
inline vec2 v2(real32 s) {
    vec2 pt = { s, s };
    return pt;
}
inline vec2 v2(real32 x, real32 y) {
    vec2 pt = { x, y };
    return pt;
}
inline vec2 v2(int32 x, int32 y) {
    vec2 pt = { static_cast<real32>(x), static_cast<real32>(y) };
    return pt;
}
inline vec2 v2(const vec2i& v) {
    vec2 pt = { (real32)v.x, (real32)v.y };
    return pt;
}

inline vec3 v3(real32 s) {
    vec3 pt = { s, s, s };
    return pt;
}
inline vec3 v3(real32 x, real32 y, real32 z) {
    vec3 pt = { x, y, z };
    return pt;
}
inline vec3 v3min(vec3& v, real32 x, real32 y, real32 z) {
    vec3 pt = {
        _Min(x, v.x),
        _Min(y, v.y),
        _Min(z, v.z) };
    return pt;
}
inline vec3 v3min(vec3& v, const vec3& m) {
    vec3 pt = {
        _Min(m.x, v.x),
        _Min(m.y, v.y),
        _Min(m.z, v.z) };
    return pt;
}
inline vec3 v3max(const vec3& v, real32 x, real32 y, real32 z) {
    vec3 pt = {
        _Max(x, v.x),
        _Max(y, v.y),
        _Max(z, v.z) };
    return pt;
}
inline vec3 v3(const vec2& v2, real32 z = 1.0f) {
    vec3 pt = { v2.x, v2.y, z };
    return pt;
}
inline vec3 v3(const vec3i& v) {
    vec3 pt = { (real32)v.x, (real32)v.y, (real32)v.z };
    return pt;
}
inline vec4 v4(__m128& s) {
    vec4 pt;
    _mm_storeu_ps(pt.E, s);
    return pt;
}
inline vec4 v4(real32 s) {
    vec4 pt = { s, s, s, 1.0f };
    return pt;
}
inline vec4 v4(real32 x, real32 y, real32 z, real32 w = 1.0f) {
    vec4 pt = { x, y, z, w };
    return pt;
}
inline vec4 v4min(vec4& v, real32 x, real32 y, real32 z, real32 w) {
    vec4 pt = {
        _Min(x, v.x),
        _Min(y, v.y),
        _Min(z, v.z),
        _Min(w, v.w) };
    return pt;
}
inline vec4 v4max(vec4& v, real32 x, real32 y, real32 z, real32 w) {
    vec4 pt = {
        _Max(x, v.x),
        _Max(y, v.y),
        _Max(z, v.z),
        _Max(w, v.w) };
    return pt;
}
inline vec4 v4(const vec3& v3, real32 w = 1.0f) {
    vec4 pt = { v3.x, v3.y, v3.z, w };
    return pt;
}
inline __m128 m128(const vec4& v) {
    __m128 result = _mm_loadu_ps(v.E);
    return (result);
}

inline GfxBox defaultRect() {
    GfxBox rc = { v3(0.0f), v3(0.0f) };
    return rc;
}

// MATH
namespace game_math {
    inline bool32 isZero(real32 s);
    inline bool32 isNotZero(real32 s);
    inline real32 square(real32 s);
    inline vec3 square(const vec3& s);
    inline vec4 square(const vec4& s);
    inline real32 squareRoot(real32 s);
    inline vec3 squareRoot(const vec3& s);
    inline vec4 squareRoot(const vec4& s);
    inline vec3 normalize(const vec3& A);
    inline void nullify(vec3& A);
    inline vec2 perp(const vec2& Va);
    inline vec2 hadamard(const vec2& Va, const vec2& Vb);
    inline vec2i hadamard(const vec2i& Va, const vec2i& Vb);
    inline real32 length(const vec2& Va);
    inline real32 lengthSq(const vec2& Va);
    inline vec3 crossProduct(const vec3& Va, const vec3& Vb);
    inline vec3 hadamard(const vec3& Va, const vec3i& Vb);
    inline vec3 hadamard(const vec3& Va, const vec3& Vb);
    inline vec3i hadamard(const vec3i& Va, const vec3i& Vb);
    inline real32 length(const vec3& Va);
    inline real32 lengthSq(const vec3& Va);
    inline real32 dotProduct(const vec4& Va, const vec4& Vb);
    inline vec4 hadamard(const vec4& Va, const vec4& Vb);
    inline vec4i hadamard(const vec4i& Va, const vec4i& Vb);
    inline real32 getAngleBetweenVectors(vec4& Va, vec4& Vb);
    inline real32 lerp(real32 A, real32 Time, real32 B);
    inline int32 clamp(int32 Min, int32 Value, int32 Max);
    inline real32 clamp(real32 Min, real32 Value, real32 Max);
    inline vec3 clamp(real32 Min, const vec3& Value, real32 Max);
    inline vec3 clamp(const vec3& Min, const vec3& Value, const vec3& Max);
    inline real32 clamp01(real32 Value);
    inline vec3 clamp01(const vec3& Value);
    inline real32 clamp01MapToRange(real32 Min, real32 Value, real32 Max);
    inline vec3 lerp(const vec3& A, real32 Time, const vec3 &B);
    inline vec4 lerp(const vec4& A, real32 Time, const vec4 &B);
    inline int32 compareVector(const vec3& Va, const vec3& Vb);
    inline int32 ceilToInt32(real32 rl);
    inline int32 floorToInt32(real32 rl);
    inline int32 roundToInt32(real32 rl);
    inline vec3i ceilToInt32(const vec3& Va);
    inline vec3i floorToInt32(const vec3& Va);
    inline vec3i roundToInt32(const vec3& Va);
    inline vec3 ceilfv(const vec3& Va);
    inline vec3 floorfv(const vec3& Va);
    inline vec3 roundfv(const vec3& Va);
    inline vec3 absolute(const vec3& Va);
    inline vec4i ceilToInt32(const vec4& Va);
    inline vec4i floorToInt32(const vec4& Va);
    inline vec4i roundToInt32(const vec4& Va);
    inline vec4 ceilfv(const vec4& Va);
    inline vec4 floorfv(const vec4& Va);
    inline vec4 roundfv(const vec4& Va);
    inline vec4 absolute(const vec4& Va);
    inline vec4 sign(const vec4& Va);
    inline vec4 computeVector(real32 magnitude, real32 angle);
    inline mat4x4 transpose(const mat4x4& src);
    inline mat4x4 identity();
    inline mat4x4 makeMatrix(const vec3& f, const vec3& u, const vec3& r, const vec3& p);
    inline mat4x4 makeMatrix(const vec4& f, const vec4& u, const vec4& r, const vec4& p);
    inline mat4x4 makeInvertedMatrix(const vec3& f, const vec3& u, const vec3& r, const vec3& p);
    inline mat4x4 makeInvertedMatrix(const vec4& f, const vec4& u, const vec4& r, const vec4& p);
    //
    inline real32 safeRatioINF(real32 dm, const real32 div) {
        if (isZero(div)) {
            return FLT_MAX;
        }
        return (dm / div);
    }
    inline real32 safeRatio0(real32 dm, const real32 div) {
        if (isZero(div)) {
            return 0.0f;
        }
        return (dm / div);
    }
    inline real32 safeRatio0(int32 dm, const int32 div) {
        if (div == 0) {
            return 0.0f;
        }
        return ((real32)dm / (real32)div);
    }
    inline vec2 safeRatio0(const vec2& dm, const vec2& div) {
        vec2 result = {
            (isZero(div.x)) ? 0.0f : (dm.x / div.x),
            (isZero(div.y)) ? 0.0f : (dm.y / div.y)
        };
        return (result);
    }
    inline vec3 ratio(const vec3& dm, const vec3& div) {
        vec3 result = {
            dm.x / div.x,
            dm.y / div.y,
            dm.z / div.z
        };
        return (result);
    }
    inline vec3 safeRatio0(const vec3& dm, const vec3& div) {
        vec3 result = {
            (isZero(div.x)) ? 0.0f : (dm.x / div.x),
            (isZero(div.y)) ? 0.0f : (dm.y / div.y),
            (isZero(div.z)) ? 0.0f : (dm.z / div.z)
        };
        return (result);
    }
    inline vec3i safeRatio0(const vec3i& dm, const vec3i& div) {
        vec3i result = {
            (div.x == 0) ? 0 : (dm.x / div.x),
            (div.y == 0) ? 0 : (dm.y / div.y),
            (div.z == 0) ? 0 : (dm.z / div.z)
        };
        return (result);
    }
    inline vec4 safeRatio0(const vec4& dm, const vec4& div) {
        vec4 result = {
            (isZero(div.x)) ? 0.0f : (dm.x / div.x),
            (isZero(div.y)) ? 0.0f : (dm.y / div.y),
            (isZero(div.z)) ? 0.0f : (dm.z / div.z),
            (isZero(div.w)) ? 0.0f : (dm.w / div.w)
        };
        return (result);
    }
    inline real32 safeRatio1(real32 dm, const real32 div) {
        if (isZero(div)) {
            return 1.0f;
        }
        return (dm / div);
    }
    inline vec2 safeRatio1(const vec2& dm, const vec2& div) {
        vec2 result = {
            (isZero(div.x)) ? 1.0f : (dm.x / div.x),
            (isZero(div.y)) ? 1.0f : (dm.y / div.y)
        };
        return (result);
    }
    inline vec3 safeRatio1(const vec3& dm, const vec3& div) {
        vec3 result = {
            (isZero(div.x)) ? 1.0f : (dm.x / div.x),
            (isZero(div.y)) ? 1.0f : (dm.y / div.y),
            (isZero(div.z)) ? 1.0f : (dm.z / div.z)
        };
        return (result);
    }
    inline vec4 safeRatio1(const vec4& dm, const vec4& div) {
        vec4 result = {
            (isZero(div.x)) ? 1.0f : (dm.x / div.x),
            (isZero(div.y)) ? 1.0f : (dm.y / div.y),
            (isZero(div.z)) ? 1.0f : (dm.z / div.z),
            (isZero(div.w)) ? 1.0f : (dm.w / div.w)
        };
        return (result);
    }

    inline vec2 barycentric(const GfxRect& rect, const vec2& pt);
    inline vec3 barycentric(const GfxBox& rect, const vec3& pt);
}

inline bool32 isEmpty(GfxRect * rc) {
    return ((game_math::isZero(rc->maxPoint.x - rc->minPoint.x) || game_math::isZero(rc->maxPoint.y - rc->minPoint.y)) ? true32 : false32);
}
inline bool32 isEmpty(GfxBox * rc) {
    return ((game_math::isZero(rc->maxPoint.x - rc->minPoint.x) || game_math::isZero(rc->maxPoint.y - rc->minPoint.y) || game_math::isZero(rc->maxPoint.z - rc->minPoint.z)) ? true32 : false32);
}

//---vec2----------------------------------------------------------------------------
inline vec2 operator -(const vec2& from, const vec2& o) {
    vec2 result = {
        from.x - o.x,
        from.y - o.y
    };
    return(result);
}
inline vec2 operator -(const vec2& from, real32 o) {
    vec2 result = {
        from.x - o,
        from.y - o
    };
    return(result);
}
inline vec2 operator +(const vec2& from, const vec2& o) {
    vec2 result = {
        from.x + o.x,
        from.y + o.y
    };
    return(result);
}
inline vec2 operator +(const vec2& from, real32 o) {
    vec2 result = {
        from.x + o,
        from.y + o
    };
    return(result);
}

inline vec2 operator -=(vec2& to, const vec2& o) {
    to.x -= o.x;
    to.y -= o.y;
    return to;
}
inline vec2 operator +=(vec2& to, const vec2& o) {
    to.x += o.x;
    to.y += o.y;
    return(to);
}

inline vec2 operator -=(vec2& to, real32 o) {
    to.x -= o;
    to.y -= o;
    return(to);
}
inline vec2 operator +=(vec2& to, real32 o) {
    to.x += o;
    to.y += o;
    return(to);
}

inline vec2 operator *(const vec2& from, real32 o) {
    vec2 result = {
        from.x * o,
        from.y * o
    };
    return(result);
}
inline vec2 operator *(real32 o, const vec2& from) {
    vec2 result = {
        from.x * o,
        from.y * o
    };
    return(result);
}

inline vec2 operator *=(vec2& to, real32 o) {
    to.x *= o;
    to.y *= o;
    return to;
}


inline vec2 operator /(const vec2& from, real32 o) {
    Assert(0 != o);
    vec2 result = {
        from.x / o,
        from.y / o
    };
    return(result);
}
inline vec2 operator /=(vec2& to, real32 o) {
    to.x /= o;
    to.y /= o;
    return to;
}

inline vec2 operator -(const vec2& o) {
    vec2 pt = {
        -(o.x),
        -(o.y)
    };
    return pt;
}

//---vec3i----------------------------------------------------------------------------
inline vec3i operator -(const vec3i& from, const vec3i& o) {
    vec3i result = {
        from.x - o.x,
        from.y - o.y,
        from.z - o.z
    };
    return(result);
}
inline vec3i operator -(const vec3i& from, int32 o) {
    vec3i result = {
        from.x - o,
        from.y - o,
        from.z - o
    };
    return(result);
}
inline vec3i operator +(const vec3i& from, const vec3i& o) {
    vec3i result = {
        from.x + o.x,
        from.y + o.y,
        from.z + o.z
    };
    return(result);
}
inline vec3i operator +(const vec3i& from, int32 o) {
    vec3i result = {
        from.x + o,
        from.y + o,
        from.z + o
    };
    return(result);
}

inline vec3i operator -=(vec3i& to, const vec3i& o) {
    to.x -= o.x;
    to.y -= o.y;
    to.z -= o.z;
    return to;
}
inline vec3i operator +=(vec3i& to, const vec3i& o) {
    to.x += o.x;
    to.y += o.y;
    to.z += o.z;
    return(to);
}

inline vec3i operator -=(vec3i& to, int32 o) {
    to.x -= o;
    to.y -= o;
    to.z -= o;
    return(to);
}
inline vec3i operator +=(vec3i& to, int32 o) {
    to.x += o;
    to.y += o;
    to.z += o;
    return(to);
}

inline vec3i operator *(const vec3i& from, int32 o) {
    vec3i result = {
        from.x * o,
        from.y * o,
        from.z * o
    };
    return(result);
}
inline vec3i operator *(int32 o, const vec3i& from) {
    vec3i result = {
        from.x * o,
        from.y * o,
        from.z * o
    };
    return(result);
}

inline vec3i operator *=(vec3i& to, int32 o) {
    to.x *= o;
    to.y *= o;
    to.z *= o;
    return to;
}

inline vec3i operator /(const vec3i& from, int32 o) {
    Assert(0 != o);
    vec3i result = {
        from.x / o,
        from.y / o,
        from.z / o
    };
    return(result);
}
inline vec3i operator /=(vec3i& to, int32 o) {
    to.x /= o;
    to.y /= o;
    to.z /= o;
    return to;
}

inline vec3i operator -(const vec3i& o) {
    vec3i pt = {
        -(o.x),
        -(o.y),
        -(o.z)
    };
    return pt;
}

//---vec3----------------------------------------------------------------------------
inline vec3 operator -(const vec3& from, const vec3& o) {
    vec3 result = {
        from.x - o.x,
        from.y - o.y,
        from.z - o.z
    };
    return(result);
}
inline vec3 operator -(const vec3& from, real32 o) {
    vec3 result = {
        from.x - o,
        from.y - o,
        from.z - o
};
    return(result);
}
inline vec3 operator +(const vec3& from, const vec3& o) {
    vec3 result = {
        from.x + o.x,
        from.y + o.y,
        from.z + o.z
    };
    return(result);
}
inline vec3 operator +(const vec3& from, real32 o) {
    vec3 result = {
        from.x + o,
        from.y + o,
        from.z + o
    };
    return(result);
}

inline vec3 operator -=(vec3& to, const vec3& o) {
    to.x -= o.x;
    to.y -= o.y;
    to.z -= o.z;
    return to;
}
inline vec3 operator +=(vec3& to, const vec3& o) {
    to.x += o.x;
    to.y += o.y;
    to.z += o.z;
    return(to);
}

inline vec3 operator -=(vec3& to, real32 o) {
    to.x -= o;
    to.y -= o;
    to.z -= o;
    return(to);
}
inline vec3 operator +=(vec3& to, real32 o) {
    to.x += o;
    to.y += o;
    to.z += o;
    return(to);
}

inline vec3 operator *(const vec3& from, real32 o) {
    vec3 result = {
        from.x * o,
        from.y * o,
        from.z * o
    };
    return(result);
}
inline vec3 operator *(real32 o, const vec3& from) {
    vec3 result = {
        from.x * o,
        from.y * o,
        from.z * o
    };
    return(result);
}

inline vec3 operator *=(vec3& to, real32 o) {
    to.x *= o;
    to.y *= o;
    to.z *= o;
    return to;
}

inline vec3 operator /(const vec3& from, real32 o) {
    Assert(0 != o);
    vec3 result = {
        from.x / o,
        from.y / o,
        from.z / o
    };
    return(result);
}
inline vec3 operator /=(vec3& to, real32 o) {
    to.x /= o;
    to.y /= o;
    to.z /= o;
    return to;
}

inline vec3 operator -(const vec3& o) {
    vec3 pt = {
        -(o.x),
        -(o.y),
        -(o.z)
    };
    return pt;
}

//---vec4----------------------------------------------------------------------------
inline vec4 operator *(const vec4& Va, real32 s) {
#ifdef USE_SIMD
    vec4 result;
    __m128 Vai = _mm_load_ps(Va.E);
    __m128 Vbi = _mm_set_ps1(s);
    __m128 v = _mm_mul_ps(Vai, Vbi);
    _mm_store_ps(result.E, v);
#else
    vec4 result = {
        Va.x * s,
        Va.y * s,
        Va.z * s,
        Va.w * s
    };
#endif
    return(result);
}
inline vec4 operator *(real32 s, const vec4& Va) {
#ifdef USE_SIMD
    vec4 result;
    __m128 Vai = _mm_load_ps(Va.E);
    __m128 Vbi = _mm_set_ps1(s);
    __m128 v = _mm_mul_ps(Vai, Vbi);
    _mm_store_ps(result.E, v);
#else
    vec4 result = {
        Va.x * s,
        Va.y * s,
        Va.z * s,
        Va.w * s
    };
#endif
    return(result);
}
inline vec4 operator /(const vec4& Va, real32 s) {
#ifdef USE_SIMD
    vec4 result;
    __m128 Vai = _mm_load_ps(Va.E);
    __m128 Vbi = _mm_set_ps1(s);
    __m128 v = _mm_div_ps(Vai, Vbi);
    _mm_store_ps(result.E, v);
#else
    vec4 result = {
        Va.x / s,
        Va.y / s,
        Va.z / s,
        Va.w / s
    };
#endif
    return(result);
}
inline vec4 operator +(const vec4& Va, const vec4& Vb) {
#ifdef USE_SIMD
    vec4 result;
    __m128 Vai = _mm_load_ps(Va.E);
    __m128 Vbi = _mm_load_ps(Va.E);
    __m128 v = _mm_add_ps(Vai, Vbi);
    _mm_store_ps(result.E, v);
#else
    vec4 result = {
        Va.x + Vb.x,
        Va.y + Vb.y,
        Va.z + Vb.z,
        Va.w + Vb.w
    };
#endif
    return(result);
}
inline vec4 operator +(const vec4& Va, real32 s) {
#ifdef USE_SIMD
    vec4 result;
    __m128 Vai = _mm_load_ps(Va.E);
    __m128 Vbi = _mm_set_ps1(s);
    __m128 v = _mm_add_ps(Vai, Vbi);
    _mm_store_ps(result.E, v);
#else
    vec4 result = {
        Va.x + s,
        Va.y + s,
        Va.z + s,
        Va.w + s
    };
#endif
    return(result);
}
inline vec4 operator -(const vec4& Va, const vec4& Vb) {
#ifdef USE_SIMD
    vec4 result;
    __m128 Vai = _mm_load_ps(Va.E);
    __m128 Vbi = _mm_load_ps(Vb.E);
    __m128 v = _mm_sub_ps(Vai, Vbi);
    _mm_store_ps(result.E, v);
#else
    vec4 result = {
        Va.x - Vb.x,
        Va.y - Vb.y,
        Va.z - Vb.z,
        Va.w - Vb.w
    };
#endif
    return(result);
}
inline vec4 operator -(const vec4& Va, real32 s) {
#ifdef USE_SIMD
    vec4 result;
    __m128 Vai = _mm_load_ps(Va.E);
    __m128 Vbi = _mm_set_ps1(s);
    __m128 v = _mm_sub_ps(Vai, Vbi);
    _mm_store_ps(result.E, v);
#else
    vec4 result = {
        Va.x - s,
        Va.y - s,
        Va.z - s,
        Va.w - s
    };
#endif
    return(result);
}

inline vec4 operator -=(vec4& to, const vec4& o) {
    to.x -= o.x;
    to.y -= o.y;
    to.z -= o.z;
    to.w -= o.w;
    return to;
}
inline vec4 operator +=(vec4& to, const vec4& o) {
    to.x += o.x;
    to.y += o.y;
    to.z += o.z;
    to.w += o.w;
    return(to);
}

inline vec4 operator -=(vec4& to, real32 o) {
    to.x -= o;
    to.y -= o;
    to.z -= o;
    to.w -= o;
    return(to);
}
inline vec4 operator +=(vec4& to, real32 o) {
    to.x += o;
    to.y += o;
    to.z += o;
    to.w += o;
    return(to);
}

inline vec4 operator *=(vec4& to, real32 o) {
    to.x *= o;
    to.y *= o;
    to.z *= o;
    to.w *= o;
    return to;
}

inline vec4 operator /=(vec4& to, real32 o) {
    to.x /= o;
    to.y /= o;
    to.z /= o;
    to.w /= o;
    return to;
}

inline vec4 operator -(const vec4& o) {
    vec4 pt = {
        -(o.x),
        -(o.y),
        -(o.z),
        -(o.w)
    };
    return pt;
}

// VECTOR COMPARE OPERATORS
inline bool32 operator ==(vec3& lf, real32 o) {
    vec3 absVector = game_math::absolute(v3(o) - lf);
    return ((absVector.x < game_math::epsilon) && (absVector.y < game_math::epsilon) && (absVector.z < game_math::epsilon)) ? true32 : false32;
}
inline bool32 operator <(vec3& l, vec3& r) {
    return (game_math::compareVector(l, r) == (-1)) ? true32 : false32;
}
inline bool32 operator ==(vec3& lf, vec3& rt) {
	vec3 absVector = game_math::absolute(rt - lf);
	return ((absVector.x < game_math::epsilon) && (absVector.y < game_math::epsilon) && (absVector.z < game_math::epsilon)) ? true32 : false32;
}
inline bool32 operator !=(vec3& lf, vec3& rt) {
	vec3 absVector = game_math::absolute(rt - lf);
	return ((absVector.x >= game_math::epsilon) || (absVector.y >= game_math::epsilon) || (absVector.z >= game_math::epsilon)) ? true32 : false32;
}

inline bool32 operator ==(vec4& lf, real32 o) {
    vec4 absVector = game_math::absolute(v4(o) - lf);
    return ((absVector.x < game_math::epsilon) && (absVector.y < game_math::epsilon) && (absVector.z < game_math::epsilon) && (absVector.w < game_math::epsilon)) ? true32 : false32;
}
inline bool32 operator <(vec4& l, vec4& r) {
    return (game_math::compareVector(l.xyz, r.xyz) == (-1)) ? true32 : false32;
}
inline bool32 operator ==(vec4& lf, vec4& rt) {
    vec4 absVector = game_math::absolute(rt - lf);
    return ((absVector.x < game_math::epsilon) && (absVector.y < game_math::epsilon) && (absVector.z < game_math::epsilon) && (absVector.w < game_math::epsilon)) ? true32 : false32;
}

//
//
inline GfxBox operator*(const GfxBox& base, real32 mul) {
    GfxBox result = {
        base.minPoint * mul,
        base.maxPoint * mul
    };
    return (result);
}

// matrix 4x4 operators
inline mat4x4 operator*(real32 As, mat4x4& B) {
#ifdef USE_SIMD
    mat4x4 result;
    __m128 A = _mm_set1_ps(As);
    result.x = _mm_mul_ps(A, B.x);
    result.y = _mm_mul_ps(A, B.y);
    result.z = _mm_mul_ps(A, B.z);
    result.w = _mm_mul_ps(A, B.w);
#else
    mat4x4 result = {
        As * B.x[0], As * B.x[1], As * B.x[2], As * B.x[3],
        As * B.y[0], As * B.y[1], As * B.y[2], As * B.y[3],
        As * B.z[0], As * B.z[1], As * B.z[2], As * B.z[3],
        As * B.w[0], As * B.w[1], As * B.w[2], As * B.w[3]
    };
#endif

    return (result);
}
inline mat4x4 operator*(mat4x4& B, real32 As) {
#ifdef USE_SIMD
    mat4x4 result;
    __m128 A = _mm_set1_ps(As);
    result.x = _mm_mul_ps(A, B.x);
    result.y = _mm_mul_ps(A, B.y);
    result.z = _mm_mul_ps(A, B.z);
    result.w = _mm_mul_ps(A, B.w);
#else
    mat4x4 result = {
        As * B.x[0], As * B.x[1], As * B.x[2], As * B.x[3],
        As * B.y[0], As * B.y[1], As * B.y[2], As * B.y[3],
        As * B.z[0], As * B.z[1], As * B.z[2], As * B.z[3],
        As * B.w[0], As * B.w[1], As * B.w[2], As * B.w[3]
    };
#endif

    return (result);
}

inline mat4x4 operator+(mat4x4& A, mat4x4& B) {
#ifdef USE_SIMD
    mat4x4 result;
    result.x = _mm_add_ps(A.x, B.x);
    result.y = _mm_add_ps(A.y, B.y);
    result.z = _mm_add_ps(A.z, B.z);
    result.w = _mm_add_ps(A.w, B.w);
#else
    mat4x4 result = {
        A.x[0] + B.x[0], A.x[1] + B.x[1], A.x[2] + B.x[2], A.x[3] + B.x[3],
        A.y[0] + B.y[0], A.y[1] + B.y[1], A.y[2] + B.y[2], A.y[3] + B.y[3],
        A.z[0] + B.z[0], A.z[1] + B.z[1], A.z[2] + B.z[2], A.z[3] + B.z[3],
        A.w[0] + B.w[0], A.w[1] + B.w[1], A.w[2] + B.w[2], A.w[3] + B.w[3]
    };
#endif

    return (result);
}

inline mat4x4& operator+=(mat4x4 &A, mat4x4& B) {
#ifdef USE_SIMD
    A.x = _mm_add_ps(A.x, B.x);
    A.y = _mm_add_ps(A.y, B.y);
    A.z = _mm_add_ps(A.z, B.z);
    A.w = _mm_add_ps(A.w, B.w);
#else
    A.x[0] = A.x[0] + B.x[0]; A.x[1] = A.x[1] + B.x[1]; A.x[2] = A.x[2] + B.x[2]; A.x[3] = A.x[3] + B.x[3];
    A.y[0] = A.y[0] + B.y[0]; A.y[1] = A.y[1] + B.y[1]; A.y[2] = A.y[2] + B.y[2]; A.y[3] = A.y[3] + B.y[3];
    A.z[0] = A.z[0] + B.z[0]; A.z[1] = A.z[1] + B.z[1]; A.z[2] = A.z[2] + B.z[2]; A.z[3] = A.z[3] + B.z[3];
    A.w[0] = A.w[0] + B.w[0]; A.w[1] = A.w[1] + B.w[1]; A.w[2] = A.w[2] + B.w[2]; A.w[3] = A.w[3] + B.w[3];
#endif

    return(A);
}

inline mat4x4 operator-(mat4x4& A, mat4x4& B) {
#ifdef USE_SIMD
    mat4x4 result;
    result.x = _mm_sub_ps(A.x, B.x);
    result.y = _mm_sub_ps(A.y, B.y);
    result.z = _mm_sub_ps(A.z, B.z);
    result.w = _mm_sub_ps(A.w, B.w);
#else
    mat4x4 result = {
        A.x[0] - B.x[0], A.x[1] - B.x[1], A.x[2] - B.x[2], A.x[3] - B.x[3],
        A.y[0] - B.y[0], A.y[1] - B.y[1], A.y[2] - B.y[2], A.y[3] - B.y[3],
        A.z[0] - B.z[0], A.z[1] - B.z[1], A.z[2] - B.z[2], A.z[3] - B.z[3],
        A.w[0] - B.w[0], A.w[1] - B.w[1], A.w[2] - B.w[2], A.w[3] - B.w[3]
    };
#endif

    return (result);
}

inline mat4x4& operator-=(mat4x4 &A, mat4x4& B) {
#ifdef USE_SIMD
    A.x = _mm_sub_ps(A.x, B.x);
    A.y = _mm_sub_ps(A.y, B.y);
    A.z = _mm_sub_ps(A.z, B.z);
    A.w = _mm_sub_ps(A.w, B.w);
#else
    A.x[0] = A.x[0] - B.x[0]; A.x[1] = A.x[1] - B.x[1]; A.x[2] = A.x[2] - B.x[2]; A.x[3] = A.x[3] - B.x[3];
    A.y[0] = A.y[0] - B.y[0]; A.y[1] = A.y[1] - B.y[1]; A.y[2] = A.y[2] - B.y[2]; A.y[3] = A.y[3] - B.y[3];
    A.z[0] = A.z[0] - B.z[0]; A.z[1] = A.z[1] - B.z[1]; A.z[2] = A.z[2] - B.z[2]; A.z[3] = A.z[3] - B.z[3];
    A.w[0] = A.w[0] - B.w[0]; A.w[1] = A.w[1] - B.w[1]; A.w[2] = A.w[2] - B.w[2]; A.w[3] = A.w[3] - B.w[3];
#endif

    return(A);
}

// OPERATOR *
inline vec4 operator*(mat4x4& A, vec4& B) {
    vec4 result;
#ifdef USE_SIMD
    __m128 tempRes;
    __m128 Bt = m128(B);

    tempRes = _mm_mul_ps(A.x, Bt);
    result.x = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.y, Bt);
    result.y = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.z, Bt);
    result.z = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.w, Bt);
    result.w = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
#else
    result.x = game_math::dotProduct(A.vx, B);
    result.y = game_math::dotProduct(A.vy, B);
    result.z = game_math::dotProduct(A.vz, B);
    result.w = game_math::dotProduct(A.vw, B);
#endif
    return (result);
}

inline mat4x4 operator*(mat4x4& A, mat4x4& B) {
    mat4x4 result;
    mat4x4 Bt = game_math::transpose(B);
#ifdef USE_SIMD
    __m128 tempRes;

    tempRes = _mm_mul_ps(A.x, Bt.x);
    result.x.m128_f32[0] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.x, Bt.y);
    result.x.m128_f32[1] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.x, Bt.z);
    result.x.m128_f32[2] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.x, Bt.w);
    result.x.m128_f32[3] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];

    tempRes = _mm_mul_ps(A.y, Bt.x);
    result.y.m128_f32[0] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.y, Bt.y);
    result.y.m128_f32[1] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.y, Bt.z);
    result.y.m128_f32[2] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.y, Bt.w);
    result.y.m128_f32[3] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];

    tempRes = _mm_mul_ps(A.z, Bt.x);
    result.z.m128_f32[0] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.z, Bt.y);
    result.z.m128_f32[1] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.z, Bt.z);
    result.z.m128_f32[2] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.z, Bt.w);
    result.z.m128_f32[3] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];

    tempRes = _mm_mul_ps(A.w, Bt.x);
    result.w.m128_f32[0] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.w, Bt.y);
    result.w.m128_f32[1] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.w, Bt.z);
    result.w.m128_f32[2] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.w, Bt.w);
    result.w.m128_f32[3] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
#else
    result.x[0] = game_math::dotProduct(A.vx, B.vx);
    result.x[1] = game_math::dotProduct(A.vx, B.vy);
    result.x[2] = game_math::dotProduct(A.vx, B.vz);
    result.x[3] = game_math::dotProduct(A.vx, B.vw);

    result.y[0] = game_math::dotProduct(A.vy, B.vx);
    result.y[1] = game_math::dotProduct(A.vy, B.vy);
    result.y[2] = game_math::dotProduct(A.vy, B.vz);
    result.y[3] = game_math::dotProduct(A.vy, B.vw);

    result.z[0] = game_math::dotProduct(A.vz, B.vx);
    result.z[1] = game_math::dotProduct(A.vz, B.vy);
    result.z[2] = game_math::dotProduct(A.vz, B.vz);
    result.z[3] = game_math::dotProduct(A.vz, B.vw);

    result.w[0] = game_math::dotProduct(A.vw, B.vx);
    result.w[1] = game_math::dotProduct(A.vw, B.vy);
    result.w[2] = game_math::dotProduct(A.vw, B.vz);
    result.w[3] = game_math::dotProduct(A.vw, B.vw);
#endif

    return (result);
}

inline mat4x4& operator*=(mat4x4& A, mat4x4& B) {
    mat4x4 Bt = game_math::transpose(B);
#ifdef USE_SIMD
    __m128 tempRes;

    tempRes = _mm_mul_ps(A.x, Bt.x);
    A.x.m128_f32[0] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.x, Bt.y);
    A.x.m128_f32[1] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.x, Bt.z);
    A.x.m128_f32[2] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.x, Bt.w);
    A.x.m128_f32[3] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];

    tempRes = _mm_mul_ps(A.y, Bt.x);
    A.y.m128_f32[0] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.y, Bt.y);
    A.y.m128_f32[1] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.y, Bt.z);
    A.y.m128_f32[2] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.y, Bt.w);
    A.y.m128_f32[3] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];

    tempRes = _mm_mul_ps(A.z, Bt.x);
    A.z.m128_f32[0] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.z, Bt.y);
    A.z.m128_f32[1] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.z, Bt.z);
    A.z.m128_f32[2] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.z, Bt.w);
    A.z.m128_f32[3] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];

    tempRes = _mm_mul_ps(A.w, Bt.x);
    A.w.m128_f32[0] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.w, Bt.y);
    A.w.m128_f32[1] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.w, Bt.z);
    A.w.m128_f32[2] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
    tempRes = _mm_mul_ps(A.w, Bt.w);
    A.w.m128_f32[3] = tempRes.m128_f32[0] + tempRes.m128_f32[1] + tempRes.m128_f32[2] + tempRes.m128_f32[3];
#else
    A.x[0] = game_math::dotProduct(A.vx, B.vx);
    A.x[1] = game_math::dotProduct(A.vx, B.vy);
    A.x[2] = game_math::dotProduct(A.vx, B.vz);
    A.x[3] = game_math::dotProduct(A.vx, B.vw);

    A.y[0] = game_math::dotProduct(A.vy, B.vx);
    A.y[1] = game_math::dotProduct(A.vy, B.vy);
    A.y[2] = game_math::dotProduct(A.vy, B.vz);
    A.y[3] = game_math::dotProduct(A.vy, B.vw);

    A.z[0] = game_math::dotProduct(A.vz, B.vx);
    A.z[1] = game_math::dotProduct(A.vz, B.vy);
    A.z[2] = game_math::dotProduct(A.vz, B.vz);
    A.z[3] = game_math::dotProduct(A.vz, B.vw);

    A.w[0] = game_math::dotProduct(A.vw, B.vx);
    A.w[1] = game_math::dotProduct(A.vw, B.vy);
    A.w[2] = game_math::dotProduct(A.vw, B.vz);
    A.w[3] = game_math::dotProduct(A.vw, B.vw);
#endif
    return (A);
}


// Quaternion
typedef union Quaternion {
    vec4 v;
    struct {
        real32
            x,
            y,
            z,
            w;
    };
} Quaternion;
inline Quaternion makeQuaternion(const vec3& n, real32 a) {
    a = (a * game_math::angle2Radians) / 2.0f;
    real32 w = cosf(a);
    Quaternion q = { v4(n * sinf(a), w) };
    return (q);
}
inline Quaternion operator -(const Quaternion& q) {
	Quaternion result = {
		-q.x,
		-q.y,
		-q.z,
		q.w
	};
	return (result);
}
inline Quaternion operator *(const Quaternion& A, const Quaternion& B) {
    // result.xyz = (A*B.w + B*A.w + game_math::crossProduct(A.xyz,B.xyz);
    // result.w = A.w*B.w - game_math::dotProduct(A.xyz, B.xyz);
	Quaternion result = {
		A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y,
		A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x,
		A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w,
		A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z
	};
	return (result);
}
inline vec4 rotate(vec4& rotationAxis, real32 theta, vec4& view) {
    Quaternion V = { v4(view.xyz, 0.0f) };
	Quaternion R = makeQuaternion(rotationAxis.xyz, theta);
	Quaternion W = (R * V) * (-R);
	return v4(W.v.xyz, view.w);
}


//---------------------------------------------------
// MATH
namespace game_math {
    inline real32 minValue(real32 n, real32 m) {
        real32 result = _Min(n, m);
        return (result);
    }
    inline real32 maxValue(real32 n, real32 m) {
        real32 result = _Max(n, m);
        return (result);
    }
    inline real32 set0or1(real32 n) {
        real32 result = (isZero(n) ? 0.0f : 1.0f);
        return (result);
    }
    inline vec3 set0or1(const vec3& n) {
        vec3 result = {
            (isZero(n.x) ? 0.0f : 1.0f),
            (isZero(n.y) ? 0.0f : 1.0f),
            (isZero(n.z) ? 0.0f : 1.0f)
        };
        return (result);
    }
    inline uint32 absolute(volatile int32 num) {
        uint32 result = abs(num);
        return (result);
    }
    inline uint64 absolute(volatile int64 num) {
        uint64 result = llabs(num);
        return (result);
    }
    inline real32 absolute(volatile real32 num) {
        real32 result = fabsf(num);
        return (result);
    }

    // DEFINE
    inline bool32 isZero(volatile real32 s) {
        bool32 result = (absolute(s) <= epsilon);
        return (result);
    }
    inline bool32 isNotZero(volatile real32 s) {
        bool32 result = (absolute(s) > epsilon);
        return (result);
    }
    inline bool32 isGreaterOrEqual(volatile real32 s, const real32 value) {
        bool32 result = ((s + epsilon * (real32(s < 0.0f) - real32(s > 0.0f))) >= value);
        return result;
    }
    inline real32 square(real32 s) {
        return(s * s);
    }
    inline vec3 square(const vec3& s) {
        vec3 result = {
            s.x * s.x,
            s.y * s.y,
            s.z * s.z
        };
        return(result);
    }
    inline vec4 square(const vec4& s) {
        vec4 result = {
            s.x * s.x,
            s.y * s.y,
            s.z * s.z,
            s.w * s.w
        };
        return(result);
    }
    inline real32 squareRoot(real32 s) {
        real32 result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(s)));
        return(result);
    }
    inline vec3 squareRoot(const vec3& s) {
        __m128 i = _mm_sqrt_ps(_mm_setr_ps(s.x, s.y, s.z, 0.0f));
        vec3 result = v4(i).xyz;
        return(result);
    }
    inline vec4 squareRoot(const vec4& s) {
        __m128 i = _mm_sqrt_ps(_mm_setr_ps(s.x, s.y, s.z, s.w));
        vec4 result = v4(i);
        return(result);
    }

    inline vec3 normalize(const vec3& A) {
        real32 m = length(A);
        if (0.0f == m) {
            return (v3(0.0f));
        }
        vec3 result = {
            A.x / m,
            A.y / m,
            A.z / m
        };
        return (result);
    }

    inline real32 nullify(const real32 A) {
        if (isZero(A)) {
            return 0.0f;
        }
        return A;
    }
    inline real32 nullify(const real32 A, const real32 d) {
        if ((absolute(A) - epsilon) <= d) {
            return 0.0f;
        }
        return A;
    }
    inline void nullify(vec3& A) {
        if (isZero(A.x)) {
            A.x = 0.0f;
        }
        if (isZero(A.y)) {
            A.y = 0.0f;
        }
        if (isZero(A.z)) {
            A.z = 0.0f;
        }
    }
    // NOTE(Roman): d must be always >= 0.0f
    inline void nullify(vec3& A, const real32 d) {
        if ((absolute(A.x) - epsilon) <= d) {
            A.x = 0.0f;
        }
        if ((absolute(A.y) - epsilon) <= d) {
            A.y = 0.0f;
        }
        if ((absolute(A.z) - epsilon) <= d) {
            A.z = 0.0f;
        }
    }

    //
    // (Va)T(Vb) - the notation of dot product (inner product) of Va and Vb,
    // VectorDotProduct(Va,Vb) returns the dot product, which is |Va| * |Vb| * Cos(a),
    //	|Va| - magnitude of Va
    //	|Vb| - magnitude of Vb
    //	(a) - angle between Va and Vb
    //-----
    inline real32 dotProduct(const vec2& Va, const vec2& Vb) {
        real32 result = Va.x * Vb.x + Va.y * Vb.y;
        return (result);
    }
    inline mat4x4 outerProduct(const vec2& Va, const vec2& Vb) {
        mat4x4 result = { 0 };
        result.x[0] = Va.x * Vb.x;
        result.x[1] = Va.x * Vb.y;
        result.y[0] = Va.y * Vb.x;
        result.y[1] = Va.y * Vb.y;
        result.z[2] = 1.0f;
        result.w[3] = 1.0f;
        return (result);
    }
    //
    // Produce a vector that is perpendicular to the Va vector
    inline vec2 perp(const vec2& Va) {
        vec2 result = {
            -Va.y,
            Va.x
        };
        return (result);
    }
    inline vec2 hadamard(const vec2& Va, const vec2& Vb) {
        vec2 result = {
            Va.x * Vb.x,
            Va.y * Vb.y
        };
        return(result);
    }
    inline vec2i hadamard(const vec2i& Va, const vec2i& Vb) {
        vec2i result = {
            Va.x * Vb.x,
            Va.y * Vb.y
        };
        return(result);
    }
    //
    // Note:
    // if we choose Vb as an identity vector, we would get the projection of Va onto Vb,
    // so if we do (Va)T(Vb) * (Vb), we would get the projection of Va on Vb as a vector.
    inline real32 length(const vec2& Va) {
        // (U)dot(U) = |U|^2
        real32 result = squareRoot(Va.x*Va.x + Va.y*Va.y);
        return(result);
    }
    inline real32 lengthSq(const vec2& Va) {
        // (U)dot(U) = |U|^2
        real32 result = Va.x*Va.x + Va.y*Va.y;
        return(result);
    }
    //-----
    inline real32 dotProduct(const vec3& Va, const vec3& Vb) {
        real32 result = Va.x * Vb.x + Va.y * Vb.y + Va.z * Vb.z;
        return (result);
    }
    inline mat4x4 outerProduct(const vec3& Va, const vec3& Vb) {
        mat4x4 result = { 0 };
        result.vx.xyz = hadamard(v3(Va.x), Vb);
        result.vy.xyz = hadamard(v3(Va.y), Vb);
        result.vz.xyz = hadamard(v3(Va.z), Vb);
        result.w[3] = 1.0f;
        return (result);
    }
    // crossProduct product, aka Vector product
    // Produce a vector that is perpendicular to the plane created by AxB
    inline vec3 crossProduct(const vec3& Va, const vec3& Vb) {
        vec3 result = {};
        result.x = (Va.y * Vb.z) - (Va.z * Vb.y);
        result.y = (Va.z * Vb.x) - (Va.x * Vb.z);
        result.z = (Va.x * Vb.y) - (Va.y * Vb.x);
        return(result);
    }
    inline vec3 hadamard(const vec3& Va, const vec3& Vb) {
        vec3 result = {
            Va.x * Vb.x,
            Va.y * Vb.y,
            Va.z * Vb.z
        };
        return(result);
    }
    inline vec3 hadamard(const vec3& Va, const vec3i& Vb) {
        vec3 result = {
            Va.x * (real32)Vb.x,
            Va.y * (real32)Vb.y,
            Va.z * (real32)Vb.z
        };
        return(result);
    }
    inline vec3i hadamard(const vec3i& Va, const vec3i& Vb) {
        vec3i result = {
            Va.x * Vb.x,
            Va.y * Vb.y,
            Va.z * Vb.z
        };
        return(result);
    }
    inline real32 length(const vec3& Va) {
        // (U)dot(U) = |U|^2
        real32 result = squareRoot(Va.x*Va.x + Va.y*Va.y + Va.z*Va.z);
        return(result);
    }
    inline real32 lengthSq(const vec3& Va) {
        // (U)dot(U) = |U|^2
        real32 result = Va.x*Va.x + Va.y*Va.y + Va.z*Va.z;
        return(result);
    }
    //-----
    inline real32 dotProduct(const vec4& Va, const vec4& Vb) {
        real32 result = Va.x * Vb.x + Va.y * Vb.y + Va.z * Vb.z + Va.w * Vb.w;
        return (result);
    }
    inline mat4x4 outerProduct(const vec4& Va, const vec4& Vb) {
        mat4x4 result = makeMatrix(
             hadamard(v4(Va.x), Vb) ,
             hadamard(v4(Va.y), Vb) ,
             hadamard(v4(Va.z), Vb) ,
             hadamard(v4(Va.w), Vb) );
        return (result);
    }
    inline vec4 hadamard(const vec4& Va, const vec4& Vb) {
        vec4 result = {
            Va.x * Vb.x,
            Va.y * Vb.y,
            Va.z * Vb.z,
            Va.w * Vb.w
        };
        return(result);
    }
    inline vec4i hadamard(const vec4i& Va, const vec4i& Vb) {
        vec4i result = {
            Va.x * Vb.x,
            Va.y * Vb.y,
            Va.z * Vb.z,
            Va.w * Vb.w
        };
        return(result);
    }
    inline real32 getAngleBetweenVectors(const vec4& Va, const vec4& Vb) {
        vec4 Ua = Va / length(Va.xyz);
        vec4 Ub = Vb / length(Vb.xyz);

        real32 dot = dotProduct(Ua.xyz, Ub.xyz);
        return acosf(dot);
    }

    inline real32 lerp(real32 A, real32 Time, real32 B) {
        real32 result = (1.0f - Time) * A + Time * B;
        return (result);
    }

    inline int32 clamp(int32 Min, int32 Value, int32 Max) {
        int32 result = _Min(Max, _Max(Value, Min));
        return (result);
    };
    inline real32 clamp(real32 Min, real32 Value, real32 Max) {
        real32 result = _Min(Max, _Max(Value, Min));
        return (result);
    };
    inline vec3 clamp(real32 Min, const vec3& Value, real32 Max) {
        vec3 result = {
            _Min(Max, _Max(Value.x, Min)),
            _Min(Max, _Max(Value.y, Min)),
            _Min(Max, _Max(Value.z, Min))
        };
        return (result);
    };
    inline vec3 clamp(const vec3& Min, const vec3& Value, const vec3& Max) {
        vec3 result = {
            _Min(Max.x, _Max(Value.x, Min.x)),
            _Min(Max.y, _Max(Value.y, Min.y)),
            _Min(Max.z, _Max(Value.z, Min.z))
        };
        return (result);
    };
    inline real32 clamp01(real32 Value) {
        real32 result = _Min(1.0f, _Max(Value, 0.0f));
        return (result);
    }
    inline vec2 clamp01(const vec2& Value) {
        vec2 result = 
        {
            _Min(1.0f, _Max(Value.x, 0.0f)),
            _Min(1.0f, _Max(Value.y, 0.0f))
        };
        return (result);
    }
    inline vec3 clamp01(const vec3& Value) {
        vec3 result = 
        {
            _Min(1.0f, _Max(Value.x, 0.0f)),
            _Min(1.0f, _Max(Value.y, 0.0f)),
            _Min(1.0f, _Max(Value.z, 0.0f))
        };
        return (result);
    }
    inline real32 clamp01MapToRange(real32 Min, real32 Value, real32 Max) {
        real32 result = 0.0f;
        real32 Range = Max - Min;
        if (Range != 0.0f) {
            result = clamp01((Value - Min) / Range);
        }

        return (result);
    }

    //--------------------------------------------------------------------------------------------------------------
    //
    rc_macro_cpp vec3 lerp(const vec3& A, real32 Time, const vec3 &B) {
        return ((1.0f - Time) * A + Time * B);
    }
    //--------------------------------------------------------------------------------------------------------------
    //
    rc_macro_cpp vec4 lerp(const vec4& A, real32 Time, const vec4 &B) {
        return ((1.0f - Time) * A + Time * B);
    }

    inline int32 compareVector(const vec3& Va, const vec3& Vb) {
        real32 aLength = length(Va);
        real32 bLength = length(Vb);
        if ((aLength - bLength) < epsilon) {
            return 0;
        }
        if (aLength > bLength) {
            return 1;
        }
        return (-1);
    }

    inline vec3 maxOf(const vec3& Va, const vec3& Vb) {
        vec3 result = {
            (Va.x - Vb.x > epsilon) ? Vb.x : Va.x,
            (Va.y - Vb.y > epsilon) ? Vb.y : Va.y,
            (Va.z - Vb.z > epsilon) ? Vb.z : Va.z
        };
        return (result);
    }

    inline vec4 maximumValue(vec4& Va, vec4& Vb) {
        __m128 Vai = { Va.x, Va.y, Va.z, Va.w };
        __m128 Vbi = { Vb.x, Vb.y, Vb.z, Vb.w };
        __m128 v = _mm_max_ps(Vai, Vbi);
        vec4 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2],
            v.m128_f32[3]
        };
        return (result);
    }

    inline vec4 minimumValue(vec4& Va, vec4& Vb) {
        __m128 Vai = { Va.x, Va.y, Va.z, Va.w };
        __m128 Vbi = { Vb.x, Vb.y, Vb.z, Vb.w };
        __m128 v = _mm_min_ps(Vai, Vbi);
        vec4 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2],
            v.m128_f32[3]
        };
        return (result);
    }

    // _MM_FROUND_NINT
#define rc_mm_round_ps(val) _mm_round_ps((val), _MM_FROUND_NINT)
    inline int32 ceilToInt32(real32 rl) {
        return static_cast<int32>(mathf::ceilf(rl)); //static_cast<real32>(static_cast<int32>(rl + 1.0f));
    }
    inline int32 floorToInt32(real32 rl) {
        return static_cast<int32>(mathf::floorf(rl)); //static_cast<real32>(static_cast<int32>(rl));
    }
    inline int32 roundToInt32(real32 rl) {
        return static_cast<int32>(mathf::roundf(rl));
    }
    inline vec3i ceilToInt32(const vec3& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, 0.0f };
        __m128 v = _mm_ceil_ps(Vai);
        vec3i result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2]
        };
#else
        vec3i result = {
            (int32)mathf::ceilf(Va.x),
            (int32)mathf::ceilf(Va.y),
            (int32)mathf::ceilf(Va.z)
        };
#endif
        return(result);
    }
    inline vec3i floorToInt32(const vec3& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, 0.0f };
        __m128 v = _mm_floor_ps(Vai);
        vec3 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2]
        };
#else
        vec3i result = {
            (int32)mathf::floorf(Va.x),
            (int32)mathf::floorf(Va.y),
            (int32)mathf::floorf(Va.z)
        };
#endif
        return(result);
    }
    inline vec3i roundToInt32(const vec3& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, 0.0f };
        __m128 v = _mm_floor_ps(Vai);
        vec3 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2]
        };
#else
        vec3i result = {
            (int32)mathf::roundf(Va.x),
            (int32)mathf::roundf(Va.y),
            (int32)mathf::roundf(Va.z)
        };
#endif
        return(result);
    }
    inline vec3 ceilfv(const vec3& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, 0.0f };
        __m128 v = _mm_ceil_ps(Vai);
        vec3i result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2]
        };
#else
        vec3 result = {
            mathf::ceilf(Va.x),
            mathf::ceilf(Va.y),
            mathf::ceilf(Va.z)
        };
#endif
        return(result);
    }
    inline vec3 floorfv(const vec3& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, 0.0f };
        __m128 v = _mm_floor_ps(Vai);
        vec3 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2]
        };
#else
        vec3 result = {
            mathf::floorf(Va.x),
            mathf::floorf(Va.y),
            mathf::floorf(Va.z)
        };
#endif
        return(result);
    }
    inline vec3 roundfv(const vec3& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, 0.0f };
        __m128 v = _mm_floor_ps(Vai);
        vec3 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2]
        };
#else
        vec3 result = {
            mathf::roundf(Va.x),
            mathf::roundf(Va.y),
            mathf::roundf(Va.z)
        };
#endif
        return(result);
    }

    inline vec3 absolute(const vec3& Va) {
#ifdef USE_SIMD
        vec3 result = {
        };
        Assert(!"Got nothing");
#else
        vec3 result = {
            fabsf(Va.x),
            fabsf(Va.y),
            fabsf(Va.z)
        };
#endif
        return(result);
    }
    inline real32 sign(real32 r) {
//        return(safeRatio1(r, absolute(r)));
        return (isZero(r) ? (0.0f) : ((r < 0.0f) ? (-1.0f) : 1.0f));
    }
    inline vec3 sign(const vec3& Va) {
//        vec3 result = safeRatio1(Va, absolute(Va));
//        return(result);
        vec3 result = {
            (isZero(Va.x) ? (0.0f) : ((Va.x < 0.0f) ? (-1.0f) : (1.0f))),
            (isZero(Va.y) ? (0.0f) : ((Va.y < 0.0f) ? (-1.0f) : (1.0f))),
            (isZero(Va.z) ? (0.0f) : ((Va.z < 0.0f) ? (-1.0f) : (1.0f)))
        };
        return (result);
    }

    inline vec4i ceilToInt32(const vec4& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, Va.w };
        __m128 v = _mm_ceil_ps(Vai);
        vec4 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2],
            v.m128_f32[3]
        };
#else
        vec4i result = {
            (int32)mathf::ceilf(Va.x),
            (int32)mathf::ceilf(Va.y),
            (int32)mathf::ceilf(Va.z),
            (int32)mathf::ceilf(Va.w)
        };
#endif
        return(result);
    }
    inline vec4i floorToInt32(const vec4& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, Va.w };
        __m128 v = _mm_floor_ps(Vai);
        vec4 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2],
            v.m128_f32[3]
        };
#else
        vec4i result = {
            (int32)mathf::floorf(Va.x),
            (int32)mathf::floorf(Va.y),
            (int32)mathf::floorf(Va.z),
            (int32)mathf::floorf(Va.w)
        };
#endif
        return(result);
    }
    inline vec4i roundToInt32(const vec4& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, Va.w };
        __m128 v = _mm_floor_ps(Vai);
        vec4 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2],
            v.m128_f32[3]
        };
#else
        vec4i result = {
            (int32)mathf::roundf(Va.x),
            (int32)mathf::roundf(Va.y),
            (int32)mathf::roundf(Va.z),
            (int32)mathf::roundf(Va.w)
        };
#endif
        return(result);
    }
    inline vec4 ceilfv(const vec4& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, Va.w };
        __m128 v = _mm_ceil_ps(Vai);
        vec4 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2],
            v.m128_f32[3]
        };
#else
        vec4 result = {
            mathf::ceilf(Va.x),
            mathf::ceilf(Va.y),
            mathf::ceilf(Va.z),
            mathf::ceilf(Va.w)
        };
#endif
        return(result);
    }
    inline vec4 floorfv(const vec4& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, Va.w };
        __m128 v = _mm_floor_ps(Vai);
        vec4 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2],
            v.m128_f32[3]
        };
#else
        vec4 result = {
            mathf::floorf(Va.x),
            mathf::floorf(Va.y),
            mathf::floorf(Va.z),
            mathf::floorf(Va.w)
        };
#endif
        return(result);
    }
    inline vec4 roundfv(const vec4& Va) {
#ifdef USE_SIMD
        __m128 Vai = { Va.x, Va.y, Va.z, Va.w };
        __m128 v = _mm_floor_ps(Vai);
        vec4 result = {
            v.m128_f32[0],
            v.m128_f32[1],
            v.m128_f32[2],
            v.m128_f32[3]
        };
#else
        vec4 result = {
            mathf::roundf(Va.x),
            mathf::roundf(Va.y),
            mathf::roundf(Va.z),
            mathf::roundf(Va.w)
        };
#endif
        return(result);
    }

    inline vec4 absolute(const vec4& Va) {
#ifdef USE_SIMD
        vec4 result = {
        };
        Assert(!"Got nothing");
#else
        vec4 result = {
            fabsf(Va.x),
            fabsf(Va.y),
            fabsf(Va.z),
            fabsf(Va.w)
        };
#endif
        return(result);
    }
    inline vec4 sign(const vec4& Va) {
        vec4 result = safeRatio1(Va, absolute(Va));
        return(result);
    }

    inline vec4 computeVector(real32 magnitude, real32 angle) {
        vec4 result = { sinf(angle)*magnitude, 0.0f, cosf(angle)*magnitude, 0.0f };
        return(result);
    }

    inline mat4x4 transpose(const mat4x4& src) {
        mat4x4 result;
        result.x[0] = src.x[0];
        result.x[1] = src.y[0];
        result.x[2] = src.z[0];
        result.x[3] = src.w[0];
        result.y[0] = src.x[1];
        result.y[1] = src.y[1];
        result.y[2] = src.z[1];
        result.y[3] = src.w[1];
        result.z[0] = src.x[2];
        result.z[1] = src.y[2];
        result.z[2] = src.z[2];
        result.z[3] = src.w[2];
        result.w[0] = src.x[3];
        result.w[1] = src.y[3];
        result.w[2] = src.z[3];
        result.w[3] = src.w[3];
        return (result);
    }

    inline mat4x4 identity() {
        mat4x4 result = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        return (result);
    }

    inline mat4x4 makeMatrix(const vec3& f, const vec3& u, const vec3& r, const vec3& p) {
        mat4x4 result = {
            f.x, u.x, r.x, p.x,
            f.y, u.y, r.y, p.y,
            f.z, u.z, r.z, p.z,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        return (result);
    }
    inline mat4x4 makeMatrix(const vec4& f, const vec4& u, const vec4& r, const vec4& p) {
        mat4x4 result = {
            f.x, u.x, r.x, p.x,
            f.y, u.y, r.y, p.y,
            f.z, u.z, r.z, p.z,
            f.w, u.w, r.w, p.w
        };
        return (result);
    }

    inline mat4x4 makeInvertedMatrix(const vec3& f, const vec3& u, const vec3& r, const vec3& p) {
        mat4x4 result = {
            f.x, u.x, r.x, p.x,
            f.y, u.y, r.y, p.y,
            f.z, u.z, r.z, p.z,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        return (result);
    }
    inline mat4x4 makeInvertedMatrix(const vec4& f, const vec4& u, const vec4& r, const vec4& p) {
        mat4x4 result = {
            f.x, u.x, r.x, p.x,
            f.y, u.y, r.y, p.y,
            f.z, u.z, r.z, p.z,
            f.w, u.w, r.w, p.w
        };
        return (result);
    }

    // barycentric coordinate will have all of the axis coords as (0 <= result <= 1) if pt is inside rect
    inline vec2 barycentric(const GfxRect& rect, const vec2& pt) {
        vec2 result = safeRatio0(pt - rect.minPoint, rect.maxPoint - rect.minPoint);
        return (result);
    }
    inline vec3 barycentric(const GfxBox& rect, const vec3& pt) {
        vec3 result = safeRatio0(pt - rect.minPoint, rect.maxPoint - rect.minPoint);
        return (result);
    }
}

#define __RC_X_MATH__H_
#endif//__RC_X_MATH__H_
