/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_FUNCTIONS_HDR_H_
#include "RaidCore_Common.h"
#include "RaidCore_Math.h"

#pragma warning(disable:4200)


rc_macro_h GfxBox& operator |=(GfxBox& to, const GfxBox& with); // merge rects
rc_macro_h GfxRect& operator |=(GfxRect& to, const GfxRect& with); // merge rects
rc_macro_h GfxBox operator &(const GfxBox& rect1, const GfxBox& rect2); // get intersection
rc_macro_h GfxRect operator &(const GfxRect& rect1, const GfxRect& rect2); // get intersection

rc_macro_h GfxBox& operator +=(GfxBox& to, const vec3& offset);
rc_macro_h GfxBox& operator -=(GfxBox& to, const vec3& offset);
rc_macro_h GfxBox& operator *=(GfxBox& to, const vec3& offset);
rc_macro_h GfxBox& operator /=(GfxBox& to, const vec3& offset);
rc_macro_h GfxBox operator +(GfxBox& rc, vec3& offset);
rc_macro_h GfxBox operator -(GfxBox& rc, vec3& offset);
rc_macro_h GfxBox operator *(GfxBox& rc, vec3& offset);
rc_macro_h GfxBox operator /(GfxBox& rc, vec3& offset);

namespace game_rect {
    // 3D rect (cube!)
    //
    rc_macro_h GfxBox createGfxRect(const vec3& HalfDim);
    rc_macro_h GfxBox createGfxRect(const vec3& Center, const vec3& HalfDim);
    rc_macro_h GfxBox expandTo(const GfxBox& rc, const vec3& radius);
    rc_macro_h GfxBox expandTo(const GfxBox& rc, const vec3& radiusTop, const vec3& radiusBottom);
    rc_macro_h GfxBox expandTo(const GfxBox& rc, real32 radius_w, real32 radius_h);
    rc_macro_h void expand(GfxBox& rc, const vec3& radius);
    rc_macro_h void expand(GfxBox& rc, const vec3& radiusTop, const vec3& radiusBottom);
    rc_macro_h void expand(GfxBox& rc, real32 radius_w, real32 radius_h);
    //
    rc_macro_h bool32 isPointInRect(const vec3& pt, const GfxBox& rc);
    rc_macro_h vec3 getSize(const GfxBox& rect);

    // 2D rect
    rc_macro_h GfxRect createGfxRect(const vec2& HalfDim);
    rc_macro_h GfxRect createGfxRect(const vec2& Center, const vec2& HalfDim);
    rc_macro_h GfxRect expandTo(const GfxRect& rc, const vec2& radius);
    rc_macro_h GfxRect expandTo(const GfxRect& rc, const vec2& radiusTop, const vec2& radiusBottom);
    rc_macro_h GfxRect expandTo(const GfxRect& rc, real32 radius_w, real32 radius_h);
    rc_macro_h void expand(GfxRect& rc, const vec2& radius);
    rc_macro_h void expand(GfxRect& rc, const vec2& radiusTop, const vec2& radiusBottom);
    rc_macro_h void expand(GfxRect& rc, real32 radius_w, real32 radius_h);
    //
    rc_macro_h bool32 isPointInRect(const vec2& pt, const GfxRect& rc);
    rc_macro_h vec2 getSize(const GfxRect& rect);
    rc_macro_h bool32 hasArea(const GfxRect& rect);
    //
    rc_macro_h vec3 maxSize(const vec3& size1, const vec3& size2);
    rc_macro_h vec3 minSize(const vec3& size1, const vec3& size2);
    //
    bool32 getLineIntersection(real32 minPt, real32 maxPt, real32 org, real32 ray, real32& fmin, real32& fmax);
    bool32 getIntersection(const GfxRect& aarb, const vec2& org, const vec2& ray, real32& fRayFraction);
    bool32 getIntersection(const GfxBox& aarb, const vec3& org, const vec3& ray, real32& fRayFraction);
    bool32 testCollision(const GfxRect& aarb, const vec2& org, const vec2& ray);
    bool32 testCollision(const GfxBox& aarb, const vec3& org, const vec3& ray);
    bool32 getCollision(const GfxRect& aarb, const vec2& org, const vec2& ray, real32& fRayFraction, vec2& absorbtionVec);
    bool32 getCollision(const GfxBox& aarb, const vec3& org, const vec3& ray, real32& fRayFraction, vec3& absorbtionVec);
}

rc_macro_cpp vec4 getColumn(const mat4x4& m, uint32 col) {
    vec4 result = {
        m.E[0][col], m.E[1][col], m.E[2][col], m.E[3][col]
    };
    return (result);
}
rc_macro_cpp mat4x4 columnMatrix(const vec3& x, const vec3& y, const vec3& z) {
    mat4x4 result = {
        x.x, y.x, z.x, 0.0f,
        x.y, y.y, z.y, 0.0f,
        x.z, y.z, z.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return (result);
}
rc_macro_cpp mat4x4 rowMatrix(const vec3& x, const vec3& y, const vec3& z) {
    mat4x4 result = {
        x.x, x.y, x.z, 0.0f,
        y.x, y.y, y.z, 0.0f,
        z.x, z.y, z.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return (result);
}
rc_macro_cpp void translate(mat4x4& m, vec3& v) {
    m.x[3] += v.x;
    m.y[3] += v.y;
    m.z[3] += v.z;
}
rc_macro_cpp void rotate(mat4x4& m, real32 a, vec4& v) {
    Assert(fabs(game_math::length(v.xyz) - 1.0f) < game_math::epsilon);

    real32 c = cosf(a * game_math::angle2Radians);
    real32 s = sinf(a * game_math::angle2Radians);
    real32 t = 1.0f - c;

    m.x[0] = v.x*v.x*t + c;
    m.x[1] = v.x*v.y*t - v.z*s;
    m.x[2] = v.x*v.z*t + v.y*s;

    m.y[0] = v.y*v.x*t + v.z*s;
    m.y[1] = v.y*v.y*t + c;
    m.y[2] = v.y*v.z*t - v.x*s;

    m.z[0] = v.z*v.x*t - v.y*s;
    m.z[1] = v.z*v.y*t + v.x*s;
    m.z[2] = v.z*v.z*t + c;
}
rc_macro_cpp void scale(mat4x4& m, vec4& v) {
    m.x[0] = v.x;
    m.y[1] = v.y;
    m.z[2] = v.z;
}
rc_macro_cpp void reflect(mat4x4& m, vec4& v) {
    Assert(fabsf(game_math::length(v.xyz)) < game_math::epsilon);

    m.x[0] = 1.0f - 2.0f * v.x * v.x;
    m.y[1] = 1.0f - 2.0f * v.y * v.y;
    m.z[2] = 1.0f - 2.0f * v.z * v.z;
    m.y[0] = m.x[1] = -2.0f * v.x * v.y;
    m.z[0] = m.x[2] = -2.0f * v.x * v.z;
    m.y[2] = m.z[1] = -2.0f * v.y * v.z;
}

internal inv4x4 cameraTransform(const vec3& X, const vec3& Y, const vec3& Z, const vec3& P) {
    inv4x4 Result;

    // TODO(Roman): It seems really suspicious that unary negation binds first
    // to the m4x4... is that actually the C++ grammar?  I guess it is :(
    mat4x4 A = rowMatrix(X, Y, Z);
    vec4 AP = -(A*v4(P, 1.0f));
    translate(A, AP.xyz);
    Result.set = A;

    vec3 iX = X / game_math::lengthSq(X);
    vec3 iY = Y / game_math::lengthSq(Y);
    vec3 iZ = Z / game_math::lengthSq(Z);
    vec3 iP = { AP.x*iX.x + AP.y*iY.x + AP.z*iZ.x,
        AP.x*iX.y + AP.y*iY.y + AP.z*iZ.y,
        AP.x*iX.z + AP.y*iY.z + AP.z*iZ.z };

    mat4x4 B = columnMatrix(iX, iY, iZ);
    translate(B, -iP);
    Result.inverse = B;

    return(Result);
}

#if 0
rc_macro_cpp inv4x4 perspectiveProjection(f32 AspectWidthOverHeight, f32 focalLength, f32 NearClipPlane, f32 FarClipPlane) {
    f32 a = 1.0f;
    f32 b = AspectWidthOverHeight;
    f32 c = focalLength; // NOTE(Roman): focalLength == 1 / tan(O/2)

    f32 n = NearClipPlane; // NOTE(Roman): Near clip plane _distance_
    f32 f = FarClipPlane; // NOTE(Roman): Far clip plane _distance_

    // NOTE(Roman): These are the perspective correct terms, for when you divide by -z
    f32 d = (n + f) / (n - f);
    f32 e = (2 * f*n) / (n - f);

    inv4x4 Result =
    {
        // NOTE(Roman): Forward
        { {
            { a*c, 0, 0, 0 },
            { 0, b*c, 0, 0 },
            { 0, 0, d, e },
            { 0, 0, -1, 0 } } },

            // NOTE(Roman): Inverse
            { {
                { 1 / (a*c), 0, 0, 0 },
                { 0, 1 / (b*c), 0, 0 },
                { 0, 0, 0, -1 },
                { 0, 0, 1 / e, d / e } } },
    };

    return(Result);
}
#endif

rc_macro_cpp mat4x4 perspectiveProjection(f32 aspect, f32 focalLength, f32 zNear, f32 zFar) {
    f32 const tanHalfFovy = tanf(focalLength / 2.0f);

    // NOTE(Roman): These are the perspective correct terms, for when you divide by -z
    f32 d = (zFar + zNear) / (zFar - zNear);
    f32 e = (2.0f * zFar * zNear) / (zFar - zNear);

    mat4x4 Result = { {
        // NOTE(Roman): Forward
        { 1.0f / (aspect * tanHalfFovy), 0, 0, 0 },
        { 0, 1.0f / tanHalfFovy, 0, 0 },
        { 0, 0, d, e },
        { 0, 0, 1, 0 }
    } };
    return(Result);
}

inline mat4x4 orthographicProjection(f32 width, f32 height) {
    mat4x4 Result = { {
        { 2.0f / width, 0, 0, 0 },
        { 0, 2.0f / height, 0, 0 },
        { 0, 0, 1.0f, 0 },
        { -1.0f, -1.0f, 0, 1.0f }
    } };
    return(Result);
}

//-----------------------------------------------------------------//
// GfxBox/GfxRect operators
//
rc_macro_cpp GfxBox& operator |=(GfxBox& to, const GfxBox& with) { // merge rects
    const vec3 lCenter = 0.5f * (to.maxPoint + with.minPoint);
    const vec3 rCenter = 0.5f * (to.maxPoint + with.minPoint);

    if (lCenter.x > rCenter.x) { // to is left of with
        to.minPoint.x = to.maxPoint.x;
        to.maxPoint.x = with.minPoint.x;
    } else {
        to.minPoint.x = with.maxPoint.x;
        to.maxPoint.x = to.minPoint.x;
    }
    if (lCenter.y > rCenter.y) { // to is under of with
        to.minPoint.y = to.maxPoint.y;
        to.maxPoint.y = with.minPoint.y;
    } else {
        to.minPoint.y = with.maxPoint.y;
        to.maxPoint.y = to.minPoint.y;
    }
    if (lCenter.z > rCenter.z) { // to is in front of with
        to.minPoint.z = to.maxPoint.z;
        to.maxPoint.z = with.minPoint.z;
    } else {
        to.minPoint.z = with.maxPoint.z;
        to.maxPoint.z = to.minPoint.z;
    }
    return (to);
}
rc_macro_cpp GfxRect& operator |=(GfxRect& to, const GfxRect& with) { // merge rects
    const vec2 lCenter = 0.5f * (to.maxPoint + with.minPoint);
    const vec2 rCenter = 0.5f * (to.maxPoint + with.minPoint);

    if (lCenter.x > rCenter.x) { // to is left of with
        to.minPoint.x = to.maxPoint.x;
        to.maxPoint.x = with.minPoint.x;
    } else {
        to.minPoint.x = with.maxPoint.x;
        to.maxPoint.x = to.minPoint.x;
    }
    if (lCenter.y > rCenter.y) { // to is under of with
        to.minPoint.y = to.maxPoint.y;
        to.maxPoint.y = with.minPoint.y;
    } else {
        to.minPoint.y = with.maxPoint.y;
        to.maxPoint.y = to.minPoint.y;
    }
    return (to);
}

rc_macro_cpp GfxBox& operator +=(GfxBox& to, const vec3& offset) {
    to.minPoint += offset;
    to.maxPoint += offset;
    return to;
}
rc_macro_cpp GfxBox& operator -=(GfxBox& to, const vec3& offset) {
    to.minPoint -= offset;
    to.maxPoint -= offset;
    return to;
}
rc_macro_cpp GfxBox& operator *=(GfxBox& to, const vec3& offset) {
    to.minPoint = game_math::hadamard(to.minPoint, offset);
    to.maxPoint = game_math::hadamard(to.maxPoint, offset);
    return to;
}
rc_macro_cpp GfxBox& operator /=(GfxBox& to, const vec3& offset) {
    to.minPoint = game_math::safeRatio0(to.minPoint, offset);
    to.maxPoint = game_math::safeRatio0(to.maxPoint, offset);
    return to;
}
rc_macro_cpp GfxBox operator +(GfxBox& rc, vec3& offset) {
    GfxBox to = rc;
    to.minPoint += offset;
    to.maxPoint += offset;
    return to;
}
rc_macro_cpp GfxBox operator -(GfxBox& rc, vec3& offset) {
    GfxBox to = rc;
    to.minPoint -= offset;
    to.maxPoint -= offset;
    return to;
}
rc_macro_cpp GfxBox operator *(GfxBox& to, const vec3& offset) {
    GfxBox result = {
        game_math::hadamard(to.minPoint, offset),
        game_math::hadamard(to.maxPoint, offset) 
    };
    return (result);
}
rc_macro_cpp GfxBox operator /(GfxBox& to, const vec3& offset) {
    GfxBox result;
    result.minPoint = game_math::safeRatio0(to.minPoint, offset);
    result.maxPoint = game_math::safeRatio0(to.maxPoint, offset);
    return (result);
}


// High order operators, 
// working on compound elements, like TileCoord
//-----------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//
namespace game_rect {
    rc_macro_cpp GfxBox createGfxRect(const vec3& HalfDim) {
        GfxBox rc;
        rc.minPoint = -HalfDim;
        rc.maxPoint = HalfDim;
        return rc;
    }
    rc_macro_cpp GfxBox createGfxRect(const vec3& Center, const vec3& HalfDim) {
        GfxBox rc;
        rc.minPoint = Center - HalfDim;
        rc.maxPoint = Center + HalfDim;
        return rc;
    }
    rc_macro_cpp vec3 getDimentions(const GfxBox& rc) {
        vec3 result = rc.maxPoint - rc.minPoint;
        return (result);
    }
    rc_macro_cpp vec3 getCenter(const GfxBox& rc) {
        vec3 result = 0.5f * (rc.maxPoint + rc.minPoint);
        return (result);
    }
    rc_macro_cpp GfxBox expandTo(const GfxBox& rc, const vec3& radius) {
        GfxBox reply;
        reply.minPoint = rc.minPoint - radius;
        reply.maxPoint = rc.maxPoint + radius;
        return reply;
    }
    rc_macro_cpp GfxBox expandTo(const GfxBox& rc, const vec3& radiusTop, const vec3& radiusBottom) {
        GfxBox reply;
        reply.minPoint = rc.minPoint - radiusTop;
        reply.maxPoint = rc.maxPoint + radiusBottom;
        return reply;
    }
    rc_macro_cpp GfxBox expandTo(const GfxBox& rc, real32 radius_w, real32 radius_h, real32 radius_d) {
        GfxBox reply;
        vec3 radius = v3(radius_w, radius_h, radius_d);
        reply.minPoint = rc.minPoint - radius;
        reply.maxPoint = rc.maxPoint + radius;
        return reply;
    }
    rc_macro_cpp void expand(GfxBox& rc, const vec3& radius) {
        rc.minPoint = rc.minPoint - radius;
        rc.maxPoint = rc.maxPoint + radius;
    }
    rc_macro_cpp void expand(GfxBox& rc, const vec3& radiusTop, const vec3& radiusBottom) {
        rc.minPoint = rc.minPoint - radiusTop;
        rc.maxPoint = rc.maxPoint + radiusBottom;
    }
    rc_macro_cpp void expand(GfxBox& rc, real32 radius) {
        vec3 exp = v3(radius);
        rc.minPoint = rc.minPoint - exp;
        rc.maxPoint = rc.maxPoint + exp;
    }
    rc_macro_cpp void expand(GfxBox& rc, real32 radius_w, real32 radius_h, real32 radius_d) {
        vec3 radius = v3(radius_w, radius_h, radius_d);
        rc.minPoint = rc.minPoint - radius;
        rc.maxPoint = rc.maxPoint + radius;
    }
	rc_macro_cpp GfxBox offsetTo(const GfxBox& rc, real32 x, real32 y, real32 z) {
		GfxBox reply;
		vec3 offset = v3(x, y, z);
		reply.minPoint = rc.minPoint + offset;
		reply.maxPoint = rc.maxPoint + offset;
		return reply;
	}
	rc_macro_cpp void offset(GfxBox& rc, real32 x, real32 y, real32 z) {
		vec3 offset = v3(x, y, z);
		rc.minPoint = rc.minPoint + offset;
		rc.maxPoint = rc.maxPoint + offset;
	}

    //--------------------------------------------------------------------------------------------------------------
    //
    rc_macro_cpp bool32 isPointInRect(const vec3& pt, const GfxBox& rc) {
        if (rc.maxPoint.x >= pt.x && pt.x >= rc.minPoint.x) {
            if (rc.maxPoint.y >= pt.y && pt.y >= rc.minPoint.y) {
                if (rc.maxPoint.z >= pt.z && pt.z >= rc.minPoint.z) {
                    return true32;
                }
            }
        }
        return false32;
    }
    inline vec3 getSize(const GfxBox& rect) {
        vec3 result = {
            fabsf(rect.maxPoint.x - rect.minPoint.x),
            fabsf(rect.maxPoint.y - rect.minPoint.y),
            fabsf(rect.maxPoint.z - rect.minPoint.z)
        };
        return (result);
    }
    inline vec3 maxSize(const vec3& size1, const vec3& size2) {
        vec3 result = {
            _Max(size1.x, size2.x),
            _Max(size1.y, size2.y),
            _Max(size1.z, size2.z)
        };
        return (result);
    }
    inline vec3 minSize(const vec3& size1, const vec3& size2) {
        vec3 result = {
            _Min(size1.x, size2.x),
            _Min(size1.y, size2.y),
            _Min(size1.z, size2.z)
        };
        return (result);
    }

    //--------------------------------------------------------------------------------------------------------------
    //
    rc_macro_cpp GfxRect createGfxRect(const vec2& HalfDim) {
        GfxRect rc;
        rc.minPoint = -HalfDim;
        rc.maxPoint = HalfDim;
        return rc;
    }
    rc_macro_cpp GfxRect createGfxRect(const vec2& Center, const vec2& HalfDim) {
        GfxRect rc;
        rc.minPoint = Center - HalfDim;
        rc.maxPoint = Center + HalfDim;
        return rc;
    }
    rc_macro_cpp vec2 getDimentions(const GfxRect& rc) {
        vec2 result = rc.maxPoint - rc.minPoint;
        return (result);
    }
    rc_macro_cpp vec2 getCenter(const GfxRect& rc) {
        vec2 result = 0.5f * (rc.maxPoint + rc.minPoint);
        return (result);
    }
    rc_macro_cpp GfxRect expandTo(const GfxRect& rc, const vec2& radius) {
        GfxRect reply;
        reply.minPoint = rc.minPoint - radius;
        reply.maxPoint = rc.maxPoint + radius;
        return reply;
    }
    rc_macro_cpp GfxRect expandTo(const GfxRect& rc, const vec2& radiusTop, const vec2& radiusBottom) {
        GfxRect reply;
        reply.minPoint = rc.minPoint - radiusTop;
        reply.maxPoint = rc.maxPoint + radiusBottom;
        return reply;
    }
    rc_macro_cpp GfxRect expandTo(const GfxRect& rc, real32 radius_w, real32 radius_h) {
        GfxRect reply;
        vec2 radius = v2(radius_w, radius_h);
        reply.minPoint = rc.minPoint - radius;
        reply.maxPoint = rc.maxPoint + radius;
        return reply;
    }
    rc_macro_cpp void expand(GfxRect& rc, const vec2& radius) {
        rc.minPoint = rc.minPoint - radius;
        rc.maxPoint = rc.maxPoint + radius;
    }
    rc_macro_cpp void expand(GfxRect& rc, const vec2& radiusTop, const vec2& radiusBottom) {
        rc.minPoint = rc.minPoint - radiusTop;
        rc.maxPoint = rc.maxPoint + radiusBottom;
    }
    rc_macro_cpp void expand(GfxRect& rc, real32 radius_w, real32 radius_h) {
        vec2 radius = v2(radius_w, radius_h);
        rc.minPoint = rc.minPoint - radius;
        rc.maxPoint = rc.maxPoint + radius;
    }
	rc_macro_cpp GfxRect offsetTo(const GfxRect& rc, real32 x, real32 y) {
		GfxRect reply;
		vec2 offset = v2(x, y);
		reply.minPoint += offset;
		reply.maxPoint += offset;
		return reply;
	}
	rc_macro_cpp void offset(GfxRect& rc, real32 x, real32 y) {
		vec2 offset = v2(x, y);
		rc.minPoint += offset;
		rc.maxPoint += offset;
	}

    //--------------------------------------------------------------------------------------------------------------
    //
    rc_macro_cpp bool32 isPointInRect(const vec2& pt, const GfxRect& rc) {
        if (rc.maxPoint.x >= pt.x && pt.x >= rc.minPoint.x) {
            if (rc.maxPoint.y >= pt.y && pt.y >= rc.minPoint.y) {
                return true32;
            }
        }
        return false32;
    }
    inline vec2 getSize(const GfxRect& rect) {
        vec2 result = {
            fabsf(rect.maxPoint.x - rect.minPoint.x),
            fabsf(rect.maxPoint.y - rect.minPoint.y)
        };
        return (result);
    }

    inline bool32 hasArea(const GfxRect& rect) {
        vec2 size = getSize(rect);
        return size.x > 0.f && size.y > 0.f;
    }
}

//--------------------------------------------------------------------------------------------------------------
//

//--------------------------------------------------------------------------------------------------------------
//
struct bit_scan_result {
    bool32 Found;
    uint32 Index;
};
inline bit_scan_result findLeastSignificantSetBit(uint32 Value) {
    bit_scan_result result = {};
    result.Found = _BitScanForward((unsigned long *)&result.Index, Value);
    return (result);
}

inline uint32 rotate(uint32 Value, int32 Amount) {
    uint32 result = (Amount >= 0)?_rotl(Value, Amount):_rotr(Value, Amount);
    return (result);
}

inline uint32 rotateLeft(uint32 Value, int32 Amount) {
    uint32 result = _rotl(Value, Amount);
    return (result);
}

inline uint32 rotateRight(uint32 Value, int32 Amount) {
    uint32 result = _rotr(Value, Amount);
    return (result);
}

#define __RC_X_FUNCTIONS_HDR_H_
#endif//__RC_X_FUNCTIONS_HDR_H_
