/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __GC_TGT_DEF_

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <WinSDKVer.h>
//
// first clear _WIN32_WINNT definition...
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

//
// We target windows 7!
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <SDKDDKVer.h>

#define __GC_TGT_DEF_

#endif //__GC_TGT_DEF_

