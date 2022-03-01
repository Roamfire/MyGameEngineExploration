/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RENDER_SELECT__
#define __RENDER_SELECT__
#include "Headers\RaidCore_RenderHandler.h"

#endif

#ifdef __USE_DIRECTX__
#include "./Raid_2DDxRenderDeviceImpl.cpp"
#endif

#ifdef __USE_OPENGL__
#include "./Raid_2DOglRenderDeviceImpl.cpp"
#endif

#ifdef __USE_GDI__
#include "./Raid_2DGDIRenderDeviceImpl.cpp"
#endif

