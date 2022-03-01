/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_WORLD_H_
#include "RaidCore_Common.h"
#include "RaidCore_GameWorld.h"
#include "RaidCore_MathOperators.h"

//-----------------------------------------------------------------//
// Operators and inlined functions for game world structures
//
rc_macro_h bool32 operator <(coord_system::RegionCoord& left, coord_system::RegionCoord& right);
rc_macro_h bool32 operator ==(coord_system::RegionCoord& left, coord_system::RegionCoord& right);
rc_macro_h coord_system::RegionCoord operator -(coord_system::RegionCoord& from, coord_system::RegionCoord& o);
rc_macro_h coord_system::RegionCoord operator +(coord_system::RegionCoord& from, coord_system::RegionCoord& o);
rc_macro_h vec3 getPositionOffset(const vec3& subregionDim, coord_system::RegionCoord& from, coord_system::RegionCoord& to); 
//
//-----------------------------------------------------------------//
// coord_system::RegionCoord operators
//
rc_macro_cpp bool32 operator <(coord_system::RegionCoord& left, coord_system::RegionCoord& right) {
	if ((left.regionCoord.y < right.regionCoord.y) || (left.regionCoord.x < right.regionCoord.x) || (left.regionCoord.z < right.regionCoord.z))
		return true32;
	if (left.offset.z < right.offset.z)
		return true32;
	if ((left.offset.z == right.offset.z) && (left.offset.y < right.offset.y))
		return true32;
	if ((left.offset.z == right.offset.z) && (left.offset.y == right.offset.y) && (left.offset.x < right.offset.x))
		return true32;
	return false32;
}

rc_macro_cpp bool32 operator ==(coord_system::RegionCoord& left, coord_system::RegionCoord& right) {
	return ((
		(left.regionCoord.y == right.regionCoord.y) && (left.regionCoord.x == right.regionCoord.x) && (left.regionCoord.z == right.regionCoord.z) &&
		(left.offset == right.offset)
		) ? true32 : false32);
}
// High order operators, 
// working on compound elements, like TileCoord
//-----------------------------------------------------------------//
// coord_system::RegionCoord operators
rc_macro_cpp coord_system::RegionCoord operator -(coord_system::RegionCoord& from, coord_system::RegionCoord& o) {
	coord_system::RegionCoord dst = from;
	dst.regionCoord.x -= o.regionCoord.x;
	dst.regionCoord.y -= o.regionCoord.y;

	return dst;
}
rc_macro_cpp coord_system::RegionCoord operator +(coord_system::RegionCoord& from, coord_system::RegionCoord& o) {
	coord_system::RegionCoord dst = {
		from.regionCoord.x + o.regionCoord.x,
		from.regionCoord.y + o.regionCoord.y,
		from.regionCoord.z + o.regionCoord.z,
		from.offset + o.offset
	};
	return dst;
}
rc_macro_cpp vec3 getPositionOffset(const vec3& subregionDim, coord_system::RegionCoord& from, coord_system::RegionCoord& to)
{
	vec3 pt = v3(0.0f);
	pt.x = static_cast<real32>(static_cast<int32>(from.regionCoord.x - to.regionCoord.x) * subregionDim.x);
	pt.y = static_cast<real32>(static_cast<int32>(from.regionCoord.y - to.regionCoord.y) * subregionDim.y);
	pt.z = static_cast<real32>(static_cast<int32>(from.regionCoord.z - to.regionCoord.z) * subregionDim.z);
	pt += (from.offset - to.offset);
	return pt;
}

#define __RC_X_WORLD_H_
#endif//__RC_X_WORLD_H_