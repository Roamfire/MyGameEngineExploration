/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef _RC_XFORM_UTILS__
#include "RaidCore_MathOperators.h"

namespace coord_system {
    static const real32 coordMinOffset = 0.0001f;
    //
    // NOTE(Roman): RegionCoord is the absolute world location,
    //  regionCoord is the region location in the world in terms of x,y,z
    //  offset is the offset from the center of the region where the entity is located.
    struct RegionCoord {
        vec3i
            regionCoord;
        vec3
            offset;
    };

    rc_macro_h RegionCoord defaultRegionCoord(const vec3i& region);
    
    // get distance between region A and B, same as A-B
    rc_macro_h vec3 getDistance(const RegionCoord& A, const RegionCoord& B, const vec3& regionDim);
    rc_macro_h void offsetDistanceByBounds(const GfxVolume& v, vec3& distance);
    //
    //------------------------------------------------------------------------------------------------------------------------
    // offset: distance of change
    rc_macro_h void offsetRegionCoord(const vec3& offset, const vec3& regionDim, RegionCoord* coord);

    /*rc_macro_h void offsetRegionCoord(const vec3& offset, const vec3& regionDim, RegionCoord* coord);*/
    //------------------------------------------------------------------------------------------------------------------------
    //
}

#define _RC_XFORM_UTILS__
#endif
