/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_RENDER_HANDLER_H_
#include "RaidCore_GameWorld.h"

enum RaidCore_EDisplayWindowState {
	e_displayState_Windowed,
	e_displayState_WindowedFullScreen,
	e_displayState_FullScreen
};

#include "RaidCore_Common.h"
#include "RaidCore_Math.h"

//////////////////////////////////////////////////////////////////////////////
union GfxColor01
{
    real32 E[4];
    struct {
        real32
            r,
            g,
            b,
            a;
    };
};
union GfxColor {
    real32 E[4];
    vec4 rgba;
    struct {
        vec3
            rgb;
        real32
            _ignore0;
    };
    struct {
        real32
            r,
            g,
            b,
            a;
    };
};
inline GfxColor rgba(uint32 r, uint32 g, uint32 b, uint32 a = 255) {
    GfxColor result = {
        (real32)r, (real32)g, (real32)b, (real32)a
    };
    return (result);
}
inline GfxColor rgba01(real32 r, real32 g, real32 b, real32 a) {
    GfxColor result = {
        r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f
    };
    return (result);
}
inline GfxColor rgba01(const GfxColor& rgba) {
    GfxColor result = {
        rgba.r / 255.0f, rgba.g / 255.0f, rgba.b / 255.0f, rgba.a / 255.0f
    };
    return (result);
}
inline vec4 premultipliedAlphaLerp(const vec4& source, real32 alpha, const vec4& destination) {
    vec4 result = {
        source.x + (1.f - alpha) * destination.x,
        source.y + (1.f - alpha) * destination.y,
        source.z + (1.f - alpha) * destination.z,
        source.w + (1.f - alpha) * destination.w
    };
    return (result);
}

namespace render_const {
    extern __m128 Inv255_4x;
    extern __m128i MaskFF;
    extern __m128i MaskFFFF;
    extern __m128i MaskFF00FF;
    extern __m128 One;
    extern __m128 Zero;
    extern __m128 One255_4x;

    extern const vec2
        screenUnitDimention; // STATIC value set at game start, equal to 16x9 for a 1920x1080 screen
    extern const vec2
        viewportPixelDimention; // STATIC value set at game start, equal to 1920x1080 screen

    extern const real32 monitorWidth;

    extern GfxColor defaultColor;
}

inline GfxColor rgba_unpack4x8(uint32 c) {
    GfxColor result = {
        (real32)(c & 0xff),
        (real32)((c >> 8) & 0xff),
        (real32)((c >> 16) & 0xff),
        (real32)((c >> 24) & 0xff)
    };
    return (result);
}
inline GfxColor bgra_unpack4x8(uint32 c) {
    GfxColor result = {
        (real32)((c >> 16) & 0xff),
        (real32)((c >> 8) & 0xff),
        (real32)(c & 0xff),
        (real32)((c >> 24) & 0xff)
    };
    return (result);
}
inline uint32 bgra_pack4x8(GfxColor& v) {
    uint32 result = ((game_math::roundToInt32(v.a) << 24) |
                        (game_math::roundToInt32(v.r) << 16) |
                        (game_math::roundToInt32(v.g) << 8) |
                        game_math::roundToInt32(v.b));
    return (result);
}
inline uint32 rgba_pack4x8(GfxColor& v) {
    uint32 result = ((game_math::roundToInt32(v.a) << 24) |
                        (game_math::roundToInt32(v.b) << 16) |
                        (game_math::roundToInt32(v.g) << 8) |
                        game_math::roundToInt32(v.r));
    return (result);
}

struct UnitTranslation
{
	vec3
		translation_Vector;
};

//////////////////////////////////////////////////////////////////////////////
#include "RaidCore_UtilsHeader.h"

//////////////////////////////////////////////////////////////////////////////
struct GameState;

//////////////////////////////////////////////////////////////////////////////
struct GameAssets;
namespace game_render_engine
{
    enum gfx_texture_type {
        textureType_none = 0,
        textureType_inMemory,
        textureType_inGpu
    };

    struct GfxImageSetting {
        union {
            vec2i size;
            struct {
                int32
                    width;
                int32
                    height;
            };
        };
#if 1
        // TODO(Roman): We do not need this at all, width is always 4byte aligned
        int32
            pitch;
#endif
        uint32 bytesPerPixel;
    };
    struct GfxTexture
	{
        GfxImageSetting
            imageStats;
        vec2
            alignPercentage; // hotspot = game_math::hadamard(alignPercentage, v2(size))
        real32 widthOverHeight; // ration between width and height
        //
        uint8*
            data; // raw data, always RGBA format
        uint32
            sampleId;
        uint32
            textureId; // valid only if GPU has the image loaded in memory
        // type
        gfx_texture_type
            type;
	};
	inline uint32 getBitmapSize(const GfxImageSetting& bmp, uint32 bpp = 32)
	{
		// NOTE(Roman): All our in-game bitmaps are RGBA, using 32 bits. So, each pixel is 4 bytes and bpp is defaulted to 32
		return ((bmp.width * (bpp >> 3)) * bmp.height);
	}

    // offscreen buffer, a temporary draw area,
#pragma pack(push,4)
    struct CODE_ALIGN(4) GfxOffscreenBuffer {
        uint8*
            buffer;
        GfxImageSetting
            imageStats;
    };
#pragma pack(pop)
    //
    //
    extern RaidCore_EDisplayWindowState m_displayState;
    //
    //
    enum render_group_entry_type {
        RenderGroupEntryType_RenderEntryNoOP,
        RenderGroupEntryType_RenderEntryClear,
        RenderGroupEntryType_RenderEntryBitmap,
        RenderGroupEntryType_RenderEntryRect,
        RenderGroupEntryType_RenderEntryLine,
    };

#pragma pack(push, 1)
    struct RenderGroupEntryHeader {
        render_group_entry_type type;
        uint8 length;
    };
#pragma pack(pop)

    struct RenderEntryNoOP {
        uint32 skipOffset;
    };
    struct RenderEntryClear {
        GfxColor color;
    };

    struct render_entry_saturation {
        real32 level;
    };
    //
    // a generic transform for worldspace (3d) to viewspace (2d in units)
    struct RenderEntryTransform {
        vec3 p; // position/translation vector!!!
        real32 scale;
        bool32 valid;
    };
    //
    struct RenderEntryBitmap {
        GfxTexture* bitmap;
        GfxColor color; // base color (affects the image???)
        RenderEntryTransform
            transform;
    };

    struct RenderEntryRect {
        GfxColor color;
        bool32 fill;
        vec2 dim;

        RenderEntryTransform
            transform;
    };

    struct RenderTransform {
        bool32 Orthographic;

        // NOTE(Roman): Camera parameters
        real32 unitsToPixels; // NOTE(Roman): This translates meters _on the monitor_ into pixels _on the monitor_
        vec2 screenCenter;

        real32 focalLength;
        real32 distanceAboveTarget;

        vec3 offset;
        real32 scale;
    };

    struct RenderGroup {
        GameAssets *assets;
        real32 renderAlpha;

        uint32 GenerationID;

        vec2 MonitorHalfDimInMeters;

        RenderTransform transform;
        GfxOffscreenBuffer* drawTarget;

        intptr MaxPushBufferSize;
        intptr PushBufferSize;
        uint8 *PushBufferBase;

        uint32 MissingResourceCount;
        bool32 RendersInBackground;

        bool32 InsideRender;
    };

	//----------------------------------------------------------------------------------------------------------
	// The entity world to view translation
	// The entity draw area sorting, rect splitting, and positioning
    // 
	// first we need the hash map
	//----------------------------------------------------------------------------------------------------------
    struct PlatformWindow;
	struct GameRenderTarget;
    //
    struct RenderTargetBuffers {
        game_memory::arena_p renderBufferArena;
    };
    void initTargetBuffers(GameState* gameState, RenderTargetBuffers* targetBuffer);
    void initOffscreenBuffer(RenderTargetBuffers* targetBuffer, GfxOffscreenBuffer*& osBuffer, uint32 width, uint32 height, uint32 bpp);
    RenderGroup* createRenderGroup(GameState* gameState, game_memory::arena_p arena);
    //
    //
    bool32 initRenderer(GameState* gameState, PlatformWindow* window);
    //
    void resizeRenderer(GameState* gameState, uint32 width, uint32 height);
	//
	//
	void freeRenderTarget(GameState* gameState);
	//
	// allocate memory for a render target inside the game state
	void addRenderAsset(GameState* gameState, uint32 id);
	//
	// two functions,
	// - startRenderProcess() will setup the render state;
	//	. clear back buffer
	//	. setup rendering states like position, draw methods etc...
	RenderGroup* startRenderProcess(GameState* gameState);
	//
	// - completeRender() will finish up, launch the rendering process and flip buffers on the flip chain.
    bool32 completeRender(GameState* gameState);
    //
    //
    rc_macro_h real32 getPerspectiveScale(real32 focalLength, real32 distanceFromTarget);
    //
    // translate reletive to camera offset into screen unit space, this includes the camera rotation * translation
    RenderEntryTransform getRenderEntityTransform(RenderTransform* transform, const vec3& offset);
    //
    void perspective(RenderGroup *RenderGroup, int32 PixelWidth, int32 PixelHeight, real32 unitsToPixels, real32 focalLength, real32 distanceAboveTarget);
    void orthographic(RenderGroup *RenderGroup, int32 PixelWidth, int32 PixelHeight, real32 unitsToPixels, real32 focalLength, real32 distanceAboveTarget);

    #define pushRenderElement(Group, type) (type *)pushRenderElement_(Group, sizeof(type), RenderGroupEntryType_##type)
    void* pushRenderElement_(RenderGroup *Group, uint32 Size, render_group_entry_type Type);
    #define pushRenderChildElement(Group, type) (type *)pushRenderElement_(Group, sizeof(type), RenderGroupEntryType_##type)
    void* pushRenderChildElement_(RenderGroup *Group, uint32 Size, render_group_entry_type Type);

    void pushClear(GameState* gameState, uint32 color);
    void pushRect(RenderGroup *RenderGroup, const vec3& offset, const vec3& rectDim, bool32 fill, const GfxColor color = render_const::defaultColor);
    bool32 pushBitmap(RenderGroup *renderGroup, uint32 id, real32 height, const vec3& offset, const GfxColor color = render_const::defaultColor);
    bool32 pushSprite(RenderGroup *renderGroup, GfxTexture* tex, real32 height, const vec3& offset, const GfxColor color = render_const::defaultColor);
};

namespace game_render_scene {
    struct GfxRenderUnit {
        EntityPiece* renderSource;
        real32 distanceFromCamera;
        real32 offsetOnY;
        vec3 renderPoint;
        real32 angle;
        GfxRenderUnit* next;
    };
    // NOTE(Roman): render block is a subportion of the screen, it holds draw commands, sorted by distance to camera???
    struct GfxRenderBlock {
        GfxRenderUnit* renderEntry;
        GfxRenderBlock* next;
    };
    struct RenderSpace {
        GfxRenderBlock** viewVolumes;
        uint32 volumeCount;
        //
        GfxRenderUnit * renderRoot;
        real32 halfWidth;
        real32 halfHeight;
        uint32 nextReadOffset;
        uint32 count;
    };
}

//////////////////////////////////////////////////////////////////////////////
#ifdef __USE_OPENGL__
#undef __USE_OPENGL__
#endif
#ifdef __USE_GDI__
#undef __USE_GDI__
#endif
#ifdef __USE_DIRECTX__
#undef __USE_DIRECTX__
#endif

// NOTE(Roman): select the renderer, GDI or OPENGL
#define __USE_GDI__
//////////////////////////////////////////////////////////////////////////////
#define __RC_X_RENDER_HANDLER_H_
#endif//__RC_X_RENDER_HANDLER_H_
