/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_PHYSICS_H_
#include "RaidCore_Common.h"

namespace game_physics
{
	//
	// TerrainStopDrag- the negetive acceleration applied to any object that has stopped accelerating (no force is active)
	extern const real32 TerrainStopDrag;
	//
	// ResistanceStopDrag - the negetive acceleration applied against accelerating objects, 
	// this simulates the drag that will stop an object from accelerating into max speed (looks better when speed up is slowed towards the end)
	extern const real32 ResistanceStopDrag;
    //
    //
    extern const real32 EarthGravityAcceleration;

	//
	// Motion profile - defines how the entity moves,
	//  how it accelerates, its max speed or if it applies a turn action
	struct MotionProfile
	{
		// turning
		real32 fTurnAnglePerSecond;
		// movement
		real32 fMagnitude;
		vec3 vAccelerate;
		real32 fMaxSpeed;
		real32 fTerrainDrag;
		// decay
		real32 fAccelDecay;
		real32 fDistanceDecay; // apply terrain drag after passing this much distance
	};
	inline MotionProfile DefaultMotionProfile(real32 distance = 9999.0f)
	{
		MotionProfile p = {
			90.0f * game_math::angle2Radians,
			1.0f, // magnitude
			v3(0.0f), // accel
			5.5f, // max speed
			TerrainStopDrag * 2.f, // drag on 0 accel
			0.0f,
			distance }; // decay after a huge distance (in effect do not decay)
		return p;
	}
	inline MotionProfile ProjectileMotionProfile(real32 distance = 100.0f)
	{
		MotionProfile p = {
			0.0f,
			0.0f,
			v3(0.0f),
			30.0f,
			TerrainStopDrag * 10.0f,
			5.0f,
			distance }; // decay after a huge distance (in effect do not decay)
		return p;
	}
	inline MotionProfile SwordMotionProfile()
	{
		MotionProfile p = {
			720.0f * game_math::angle2Radians, // two spins per second
			0.0f,
			v3(0.0f),
			0.0f,
			0.0f,
			1.0f,
			0.05f }; // decay instantly
		return p;
	}

    vec3 applyMovementVector(vec3& Velocity, real32 MaxVelocity, real32 dt);
	// Parameters:
	// -  Acceleration (m/s)/s - Vector of accelaration, magnitude calculated as if from (0,0)
	// -  Velocity (m/s) - Current velocity vector, magnitude calculated as if from (0,0)
	// -  dt (s) - Time frame, in seconds (or more often, a fraction of a second)
	// Return:
	// Offset on all axis, in (m), or an offset vector... add to current position for new position.
	vec3 applyAcceleration(vec3& Acceleration, vec3& Velocity, real32 MaxVelocity, real32 dt);
	//
	// Calculate the ground drag, as it starts operating when accelaration is stopped,
	// modified by the Coefficient (0 if no terrain drag)
	vec3 calculateTerrainDrag(const vec3& Velocity, real32 Coefficient, real32 dt);

	// Calculate the position offset vector, based on:
	// - CurrentVelocity (updatedby function)
	// - CurrentAcceleration (updatedby function)
	// - TerrainDragK - This drag is applied when accelaration is stopped in any axis (and applied to that axis)
	// - MotionDragK - This drag is applied on accelaration as natural decceleration when changing directions
	vec3 getGenericMotionOffset(vec3& CurrentVelocity, vec3& CurrentAcceleration, const real32 TerrainDragK, const real32 MotionDragK, real32 MaxVelocity, real32 dTinSec);

	real32 getDistance(const vec3& Pt1, const vec3& Pt2);
}

#define __RC_X_PHYSICS_H_
#endif//__RC_X_PHYSICS_H_
