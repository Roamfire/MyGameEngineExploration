/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "../RaidCore/Headers/RaidCore_Common.h"
#include "../RaidCore/Headers/RaidCore_MathOperators.h"
//
#include "../RaidCore/Headers/RaidCore_WorldCoordinates.h"
// --

namespace coord_system {
    //
    rc_macro_cpp RegionCoord defaultRegionCoord(const vec3i& region) {
        RegionCoord pt = { region, { 0.0f, 0.0f, 0.0f } };
        return pt;
    }

    // get distance between region A and B, same as A-B
    rc_macro_cpp vec3 getDistance(const RegionCoord& A, const RegionCoord& B, const vec3& regionDim) {
        vec3i pt = A.regionCoord - B.regionCoord; // offset in region space
        vec3 result = game_math::hadamard(regionDim, pt) + (A.offset - B.offset);
        return (result);
    }
    rc_macro_cpp void offsetDistanceByBounds(const GfxVolume& v, vec3& distance) {
        distance += v.offset;
        distance -= game_math::hadamard(game_math::sign(distance), v.radialDimention);
    }
    //
    //------------------------------------------------------------------------------------------------------------------------
    // offset: distance of change
    rc_macro_cpp void offsetRegionCoord(const vec3& offset, const vec3& regionDim, RegionCoord* coord) {
        vec3 newOffset = coord->offset + offset;
        vec3 offsetSign = game_math::sign(newOffset);
        // TODO(Roman): add epsilon to region half size, so that entities do not oscilate between regions when they are exactly regionDim/2 distance from center.
        vec3 relDistance = v3max(game_math::absolute(newOffset) - (regionDim * 0.5f),
                                 0.f, 0.f, 0.f);

        vec3i offsetCorrection = game_math::floorToInt32(
            game_math::hadamard(
            game_math::ceilfv(game_math::ratio(relDistance, regionDim)), offsetSign)
            );

        coord->regionCoord += offsetCorrection;
        coord->offset = newOffset - game_math::hadamard(regionDim, offsetCorrection);
        if (coord->regionCoord.y < 0) {
            coord->regionCoord.y = 0;
            coord->offset.y = -regionDim.y * 0.5f;
        }
    }
    // offset: distance of change
    rc_macro_cpp vec3 canonizeOffset(const vec3& offset, const vec3& regionDim) {
        vec3 result;
        vec3 offsetSign = game_math::sign(offset);
        // TODO(Roman): add epsilon to region half size, so that entities do not oscilate between regions when they are exactly regionDim/2 distance from center.
        vec3 relDistance = v3max(game_math::absolute(offset) - (regionDim * 0.5f),
                                 0.f, 0.f, 0.f);

        vec3i offsetCorrection = game_math::floorToInt32(
            game_math::hadamard(
            game_math::ceilfv(game_math::ratio(relDistance, regionDim)), offsetSign)
            );

        result = offset - game_math::hadamard(regionDim, offsetCorrection);
        return (result);
    }

    //------------------------------------------------------------------------------------------------------------------------
}

//-----------------------------------------------------------------//
// vec3 operators
//

//-----------------------------------------------------------------//
// coord_system::RegionCoord operators
//

//-----------------------------------------------------------------//
// Rectangle intersection
//
rc_macro_cpp GfxBox operator &(const GfxBox& rect1, const GfxBox& rect2) {
    GfxBox rcRep;
    rcRep.minPoint.x = _Max(rect1.minPoint.x, rect2.minPoint.x);
    rcRep.minPoint.y = _Max(rect1.minPoint.y, rect2.minPoint.y);
    rcRep.minPoint.z = _Max(rect1.minPoint.z, rect2.minPoint.z);
    rcRep.maxPoint.x = _Min(rect1.maxPoint.x, rect2.maxPoint.x);
    rcRep.maxPoint.y = _Min(rect1.maxPoint.y, rect2.maxPoint.y);
    rcRep.maxPoint.z = _Min(rect1.maxPoint.z, rect2.maxPoint.z);

    return (rcRep);
}
rc_macro_cpp GfxRect operator &(const GfxRect& rect1, const GfxRect& rect2) {
    GfxRect rcRep;

    rcRep.minPoint.x = _Max(rect1.minPoint.x, rect2.minPoint.x);
    rcRep.minPoint.y = _Max(rect1.minPoint.y, rect2.minPoint.y);
    rcRep.maxPoint.x = _Min(rect1.maxPoint.x, rect2.maxPoint.x);
    rcRep.maxPoint.y = _Min(rect1.maxPoint.y, rect2.maxPoint.y);

    return (rcRep);
}

///////////////////---------------------------------------------------------//////////////////
// Fast world AARB ray to box collision
namespace {
	inline real32 Max0n(real32 a, real32 b) {
		a = _Max(a, b);
		return (a >= 0.0f) ? a : 0.0f;
	}
	inline real32 Min0n(real32 a, real32 b) {
		a = _Min(a, b);
		return (a >= 0.0f) ? a : 0.0f;
	}
	inline __m128 Max0n(__m128 a, __m128 b) {
		return _mm_max_ps(a, b);
	}
    inline __m128 Min0n(__m128 a, __m128 b) {
		return _mm_min_ps(a, b);
	}
}

rc_macro_cpp bool32 game_rect::getLineIntersection(real32 minPt, real32 maxPt, real32 v0, real32 distance, real32& fmin, real32& fmax) {
    if (game_math::isNotZero(distance)) {
        //
        real32 Ld = game_math::nullify((minPt - v0)/ distance);
        real32 Rd = game_math::nullify((maxPt - v0)/ distance);
        if (Rd < Ld) {
            Swap<real32>(&Ld, &Rd);
        }
        if ((fmin > Rd) || (fmax < Ld))
            return false32;
#if 1
        fmin = _Max(fmin, Ld);
        fmax = _Min(fmax, Rd);
#else
        fmin = Ld;
        fmax = Rd;
#endif
    }
    else {
        return ((minPt <= v0) && (v0 <= maxPt));
    }
    return (fmax >= fmin);
}
bool32 game_rect::getIntersection(const GfxRect& aarb, const vec2& org, const vec2& offset, real32& fRayFraction) {
    real32 fmin = 0.0f, fmax = 1.0f;
    if (getLineIntersection(aarb.minPoint.x, aarb.maxPoint.x, org.x, offset.x, fmin, fmax)) {
        if (getLineIntersection(aarb.minPoint.y, aarb.maxPoint.y, org.y, offset.y, fmin, fmax)) {
            fRayFraction = fmin;
            return ((fmin <= 1.0f) && (fmin > 0.0f));
        }
    }
    return false32;
}
bool32 game_rect::getIntersection(const GfxBox& aarb, const vec3& org, const vec3& offset, real32& fRayFraction) {
    real32 fmin=0.0f, fmax=1.0f;
    if (getLineIntersection(aarb.minPoint.x, aarb.maxPoint.x, org.x, offset.x, fmin, fmax)) {
        if (getLineIntersection(aarb.minPoint.y, aarb.maxPoint.y, org.y, offset.y, fmin, fmax)) {
            if (getLineIntersection(aarb.minPoint.z, aarb.maxPoint.z, org.z, offset.z, fmin, fmax)) {
                fRayFraction = fmin;
                return ((fmin <= 1.0f) && (fmin >= 0.0f));
            }
        }
    }
    return false32;
}

bool32 game_rect::testCollision(const GfxRect& aarb, const vec2& org, const vec2& offset) {
    real32 fmin = 0.0f, fmax = 1.0f;
    if (getLineIntersection(aarb.minPoint.x, aarb.maxPoint.x, org.x, offset.x, fmin, fmax)) {
        if (getLineIntersection(aarb.minPoint.y, aarb.maxPoint.y, org.y, offset.y, fmin, fmax)) {
            return ((fmin <= 1.0f) && (fmin >= 0.0f));
        }
    }
    return false32;
}
bool32 game_rect::testCollision(const GfxBox& aarb, const vec3& org, const vec3& offset) {
    real32 fmin = 0.0f, fmax = 1.0f;
    if (getLineIntersection(aarb.minPoint.x, aarb.maxPoint.x, org.x, offset.x, fmin, fmax)) {
        if (getLineIntersection(aarb.minPoint.y, aarb.maxPoint.y, org.y, offset.y, fmin, fmax)) {
            if (getLineIntersection(aarb.minPoint.z, aarb.maxPoint.z, org.z, offset.z, fmin, fmax)) {
                return ((fmin <= 1.0f) && (fmin >= 0.0f));
            }
        }
    }
    return false32;
}

bool32 game_rect::getCollision(const GfxRect& aarb, const vec2& org, const vec2& offset, real32& fRayFraction, vec2& absorbtionVec) {
    real32 fmin = 0.0f, fmax = 1.0f;
    real32 lastMin = 0.0f;
    if (getLineIntersection(aarb.minPoint.x, aarb.maxPoint.x, org.x, offset.x, fmin, fmax)) {
        if (fmin > lastMin) {
            absorbtionVec = v2(game_math::set0or1(offset.x), 0.f);
            lastMin = fmin;
        } else if (game_math::isZero(fmin - lastMin)) {
            absorbtionVec.x += game_math::set0or1(offset.x);
        }
        fmin = 0.0;
        if (getLineIntersection(aarb.minPoint.y, aarb.maxPoint.y, org.y, offset.y, fmin, fmax)) {
            if (fmin > lastMin) {
                absorbtionVec = v2(0.f, game_math::set0or1(offset.y));
                lastMin = fmin;
            } else if (game_math::isZero(fmin - lastMin)) {
                absorbtionVec.y += game_math::set0or1(offset.y);
            }
            fRayFraction = lastMin;
            return ((lastMin <= 1.0f) && (lastMin >= 0.0f));
        }
    }
    return false32;
}
bool32 game_rect::getCollision(const GfxBox& aarb, const vec3& org, const vec3& offset, real32& fRayFraction, vec3& absorbtionVec) {
    real32 fmin=0.0f, fmax=1.0f;
    real32 lastMin = 0.0f;
    if (getLineIntersection(aarb.minPoint.x, aarb.maxPoint.x, org.x, offset.x, fmin, fmax)) {
        if (fmin > lastMin) {
            absorbtionVec = v3(game_math::set0or1(offset.x), 0.f, 0.f);
            lastMin = fmin;
        }
        else if (game_math::isZero(fmin - lastMin)) {
            absorbtionVec.x += game_math::set0or1(offset.x);
        }
        fmin = 0.0;
        if (getLineIntersection(aarb.minPoint.y, aarb.maxPoint.y, org.y, offset.y, fmin, fmax)) {
            if (fmin > lastMin) {
                absorbtionVec = v3(0.f, game_math::set0or1(offset.y), 0.f);
                lastMin = fmin;
            } else if (game_math::isZero(fmin - lastMin)) {
                absorbtionVec.y += game_math::set0or1(offset.y);
            }
            fmin = 0.0;
            if (getLineIntersection(aarb.minPoint.z, aarb.maxPoint.z, org.z, offset.z, fmin, fmax)) {
                if (fmin > lastMin) {
                    absorbtionVec = v3(0.f, 0.f, game_math::set0or1(offset.z));
                    lastMin = fmin;
                } else if (game_math::isZero(fmin - lastMin)) {
                    absorbtionVec.z += game_math::set0or1(offset.z);
                }
                fRayFraction = lastMin;
                return ((lastMin <= 1.0f) && (lastMin >= 0.0f));
            }
        }
    }
    return false32;
}
