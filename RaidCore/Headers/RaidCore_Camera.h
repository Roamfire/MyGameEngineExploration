/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_CAMERA_H_
#include "RaidCore_Common.h"
#include "RaidCore_Math.h"

enum CameraState {
    CameraState_FreeRoam,
    CameraState_AreaCenter,
    CameraState_EntityFollowScroll,
    CameraState_EntityFollowBounded,

    CameraState_Count
};

//
struct GameCamera {
    coord_system::RegionCoord
        iWorldPosition;		// The absolute world map position, this is used to locate the correct map region we need for the rendering and locating
    real32 focalLength, distanceAboveTarget;
    vec3 iFacing, iPtSpeed;

    CameraState state;
};


#define __RC_X_CAMERA_H_
#endif//__RC_X_CAMERA_H_
