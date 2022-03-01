/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __GC_2DGDI_RENDER_DEVICE_H_
#ifdef __USE_GDI__

// --
#include "../RaidCore/Headers/RaidCore_RenderHandler.h"
#include <windows.h>
#include "./OSHeaders/RaidCore_Win32_Struct.h"
#include "../RaidCore/Headers/RaidCore_GameState.h"

#include "../GameHandler/debug_s.h"

#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "gdi32.lib")

void drawHLine(game_render_engine::GfxOffscreenBuffer * dest, const real32 x, const real32 y, const real32 len, GfxColor& color) {
	real32 ex = x + len;

	int32 fMinX = game_math::floorToInt32(x);
	int32 fMinY = game_math::floorToInt32(y);
	int32 fMaxX = game_math::ceilToInt32(ex);

	int32 MinX = game_math::ceilToInt32(x);
	int32 MinY = game_math::ceilToInt32(y);
	int32 MaxX = game_math::floorToInt32(ex);

	if (MinX > dest->imageStats.width) {
		MinX = dest->imageStats.width;
	}
	else if (MinX < 0) {
		MinX = 0;
	}
	if (MinY > dest->imageStats.height) {
		MinY = dest->imageStats.height;
	}
	else if (MinY < 0) {
		MinY = 0;
	}
	if (MaxX > dest->imageStats.width) {
		MaxX = dest->imageStats.width;
	}
	else if (MaxX < 0) {
		MaxX = 0;
	}
	if (fMinX > dest->imageStats.width) {
		fMinX = dest->imageStats.width;
	}
	else if (fMinX < 0) {
		fMinX = 0;
	}
	if (fMinY > dest->imageStats.height) {
		fMinY = dest->imageStats.height;
	}
	else if (fMinY < 0) {
		fMinY = 0;
	}
	if (fMaxX > dest->imageStats.width) {
		fMaxX = dest->imageStats.width;
	}
	else if (fMaxX < 0) {
		fMaxX = 0;
	}

    GfxColor Null = rgba(0U, 0U, 0U, 0U);
    GfxColor preMul = color;
    preMul.rgb *= color.a;
    GfxColor left, top, right, bottom;
	if (fMinY != MinY) {
        top.rgba = premultipliedAlphaLerp(preMul.rgba, y - (real32)fMinY, Null.rgba);
        bottom.rgba = premultipliedAlphaLerp(preMul.rgba, (real32)fMinY + 1.0f - y, Null.rgba);
	}
	else {
		top = bottom = color;
	}
	if (fMinX < MinX) {
		left.rgba = game_math::lerp(
			game_math::lerp(Null.rgba, x - (real32)fMinX, top.rgba),
			y - (real32)fMinY,
			game_math::lerp(Null.rgba, x - (real32)fMinX, bottom.rgba));
	}
	if (fMaxX > MaxX) {
		right.rgba = game_math::lerp(
			game_math::lerp(top.rgba, (real32)fMaxX - ex, Null.rgba),
			y - (real32)fMinY,
			game_math::lerp(bottom.rgba, (real32)fMaxX - ex, Null.rgba));
	}

	const uint32 rWidth = (dest->imageStats.width * dest->imageStats.bytesPerPixel);
	real32 Alpha = game_math::roundToInt32(color.a) / 255.0f;
	uint8 *Row = ((uint8 *)dest->buffer +
		MinX * dest->imageStats.bytesPerPixel +
		MinY * rWidth);

	uint32 *tPixel = (uint32 *)Row;
	uint32 *bPixel = (uint32 *)(Row + rWidth);
	for (int X = fMinX; X < fMaxX; ++X) {
		GfxColor orig = rgba_unpack4x8(*tPixel);
		real32 preMulAlpha = top.a * orig.a;
		orig = *(GfxColor*)&game_math::lerp(*(vec4*)&orig, Alpha, *(vec4*)&top);
		orig.a = preMulAlpha;
		*tPixel++ = bgra_pack4x8(orig);

		if (fMinY != MinY) {
			orig = rgba_unpack4x8(*bPixel);
			preMulAlpha = bottom.a * orig.a;
			orig = *(GfxColor*)&game_math::lerp(*(vec4*)&orig, Alpha, *(vec4*)&bottom);
			orig.a = preMulAlpha;
			*bPixel++ = bgra_pack4x8(orig);
		}
	}
}
void drawVLine(game_render_engine::GfxOffscreenBuffer * dest, const real32 x, const real32 y, const real32 len, GfxColor& color) {
	real32 ey = y + len;

	int32 fMinX = game_math::floorToInt32(x);
	int32 fMinY = game_math::floorToInt32(y);
	int32 fMaxY = game_math::ceilToInt32(ey);

	int32 MinX = game_math::ceilToInt32(x);
	int32 MinY = game_math::ceilToInt32(y);
	int32 MaxY = game_math::floorToInt32(ey);

	if (MinX > dest->imageStats.width) {
		MinX = dest->imageStats.width;
	}
	else if (MinX < 0) {
		MinX = 0;
	}
	if (MinY > dest->imageStats.height) {
		MinY = dest->imageStats.height;
	}
	else if (MinY < 0) {
		MinY = 0;
	}
	if (MaxY > dest->imageStats.height) {
		MaxY = dest->imageStats.height;
	}
	else if (MaxY < 0) {
		MaxY = 0;
	}
	if (fMinX > dest->imageStats.width) {
		fMinX = dest->imageStats.width;
	}
	else if (fMinX < 0) {
		fMinX = 0;
	}
	if (fMinY > dest->imageStats.height) {
		fMinY = dest->imageStats.height;
	}
	else if (fMinY < 0) {
		fMinY = 0;
	}
	if (fMaxY > dest->imageStats.height) {
		fMaxY = dest->imageStats.height;
	}
	else if (fMaxY < 0) {
		fMaxY = 0;
	}

    GfxColor Null = rgba(0U, 0U, 0U, 0U);
    GfxColor preMul = color;
    preMul.rgb *= color.a;

    GfxColor left, top, right, bottom;
	if (fMinX != MinX) {
		left.rgba = premultipliedAlphaLerp(preMul.rgba, x - (real32)fMinX, Null.rgba);
        right.rgba = premultipliedAlphaLerp(preMul.rgba, (real32)fMinX + 1.0f - x, Null.rgba);
	}
	else {
		left = right = color;
	}
	if (fMinY < MinY) {
		top.rgba = game_math::lerp(
			game_math::lerp(Null.rgba, y - (real32)fMinY, left.rgba),
			x - (real32)fMinX,
			game_math::lerp(Null.rgba, y - (real32)fMinY, right.rgba));
	}
	if (fMaxY > MaxY) {
		bottom.rgba = game_math::lerp(
			game_math::lerp(left.rgba, (real32)fMaxY - ey, Null.rgba),
			x - (real32)fMinX,
			game_math::lerp(right.rgba, (real32)fMaxY - ey, Null.rgba));
	}

	const uint32 rWidth = (dest->imageStats.width * dest->imageStats.bytesPerPixel);
	real32 Alpha = game_math::roundToInt32(color.a) / 255.0f;
	uint8 *Row = ((uint8 *)dest->buffer +
		MinX * dest->imageStats.bytesPerPixel +
		MinY * rWidth);

	uint32 *tPixel = (uint32 *)Row;
	uint32 *bPixel = tPixel + 1;
	for (int Y = fMinY; Y < fMaxY; ++Y) {
		GfxColor orig = rgba_unpack4x8(*tPixel);
		real32 preMulAlpha = left.a * orig.a;
		orig = *(GfxColor*)&game_math::lerp(*(vec4*)&orig, Alpha, *(vec4*)&left);
		orig.a = preMulAlpha;
		*tPixel = bgra_pack4x8(orig);
		tPixel += dest->imageStats.width;

		if (fMinX != MinX) {
			orig = rgba_unpack4x8(*bPixel);
			preMulAlpha = right.a * orig.a;
			orig = *(GfxColor*)&game_math::lerp(*(vec4*)&orig, Alpha, *(vec4*)&right);
			orig.a = preMulAlpha;
			*bPixel = bgra_pack4x8(orig);
			bPixel += dest->imageStats.width;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
void DrawRectangle(game_render_engine::GfxOffscreenBuffer * dest, const vec2& vMin, const vec2& vMax, GfxColor& color) {
	int32 fMinX = game_math::floorToInt32(vMin.x);
	int32 fMinY = game_math::floorToInt32(vMin.y);
	int32 fMaxX = game_math::ceilToInt32(vMax.x);
	int32 fMaxY = game_math::ceilToInt32(vMax.y);

	int32 MinX = game_math::ceilToInt32(vMin.x);
	int32 MinY = game_math::ceilToInt32(vMin.y);
	int32 MaxX = game_math::floorToInt32(vMax.x);
	int32 MaxY = game_math::floorToInt32(vMax.y);

	if (MinX > dest->imageStats.width) {
		MinX = dest->imageStats.width;
	}
	else if (MinX < 0) {
		MinX = 0;
	}
	if (MinY > dest->imageStats.height) {
		MinY = dest->imageStats.height;
	}
	else if (MinY < 0) {
		MinY = 0;
	}
	if (MaxX > dest->imageStats.width) {
		MaxX = dest->imageStats.width;
	}
	else if (MaxX < 0) {
		MaxX = 0;
	}
	if (MaxY > dest->imageStats.height) {
		MaxY = dest->imageStats.height;
	}
	else if (MaxY < 0) {
		MaxY = 0;
	}
	if (fMinX > dest->imageStats.width) {
		fMinX = dest->imageStats.width;
	}
	else if (fMinX < 0) {
		fMinX = 0;
	}
	if (fMinY > dest->imageStats.height) {
		fMinY = dest->imageStats.height;
	}
	else if (fMinY < 0) {
		fMinY = 0;
	}
	if (fMaxX > dest->imageStats.width) {
		fMaxX = dest->imageStats.width;
	}
	else if (fMaxX < 0) {
		fMaxX = 0;
	}
	if (fMaxY > dest->imageStats.height) {
		fMaxY = dest->imageStats.height;
	}
	else if (fMaxY < 0) {
		fMaxY = 0;
	}

	GfxColor Null = rgba(0U, 0U, 0U, 0U);
	GfxColor preMul = color;
    preMul.rgb *= color.a;

    GfxColor left = {}, top = {}, right = {}, bottom = {};
	if (fMinX < MinX) {
		//left.rgba = game_math::lerp(color.rgba, vMin.x - (real32)fMinX, Null.rgba);
        left.rgba = premultipliedAlphaLerp(preMul.rgba, vMin.x - (real32)fMinX, Null.rgba);
	}
	if (fMinY < MinY) {
        top.rgba = premultipliedAlphaLerp(preMul.rgba, vMin.y - (real32)fMinY, Null.rgba);
	}
	if (fMaxX > MaxX) {
        right.rgba = premultipliedAlphaLerp(preMul.rgba, (real32)fMaxX - vMax.x, Null.rgba);
	}
	if (fMaxY > MaxY) {
        bottom.rgba = premultipliedAlphaLerp(preMul.rgba, (real32)fMaxY - vMax.y, Null.rgba);
	}

	const uint32 rWidth = (dest->imageStats.width * dest->imageStats.bytesPerPixel);
	real32 Alpha = game_math::roundToInt32(color.a) / 255.0f;
	uint8 *Row = ((uint8 *)dest->buffer +
		MinX * dest->imageStats.bytesPerPixel +
		MinY * rWidth);

	GfxColor* inUse = &color;

	for (int Y = fMinY;
		Y < fMaxY;
		++Y) {
		uint32 *Pixel = (uint32 *)Row;
		if (fMinX < MinX) {
			GfxColor orig = rgba_unpack4x8(*Pixel);

			real32 preMulAlpha = left.a * orig.a;
			orig = *(GfxColor*)&game_math::lerp(*(vec4*)&orig, Alpha, *(vec4*)&left);
			orig.a = preMulAlpha;
			*Pixel++ = bgra_pack4x8(orig);
		}
		for (int X = fMinX;
			X < fMaxX;
			++X) {
			if (Y == MinY && fMinY < MinY) {
				inUse = &top;
			}
			if (Y == MaxY && fMaxY > MaxY) {
				inUse = &bottom;
			}
			else {
				inUse = &color;
			}
			GfxColor orig = rgba_unpack4x8(*Pixel);

			real32 preMulAlpha = inUse->a * orig.a;
			orig = *(GfxColor*)&game_math::lerp(*(vec4*)&orig, Alpha, *(vec4*)inUse);
			orig.a = preMulAlpha;
			*Pixel++ = bgra_pack4x8(orig);
		}
		if (fMaxX > MaxX) {
			GfxColor orig = rgba_unpack4x8(*Pixel);

			real32 preMulAlpha = right.a * orig.a;
			orig = *(GfxColor*)&game_math::lerp(*(vec4*)&orig, Alpha, *(vec4*)&right);
			orig.a = preMulAlpha;
			*Pixel++ = bgra_pack4x8(orig);
		}
		Row += rWidth;
	}
}

struct Rect {
    vec2i
        minPoint, maxPoint;
};
void DrawRectangleQuickly(game_render_engine::GfxOffscreenBuffer* Buffer, vec2 Origin, vec2 XAxis, vec2 YAxis, vec4 Color,
    game_render_engine::GfxTexture* Texture, GfxRect ClipRect, bool32 Even)
{
    // NOTE(casey): Premultiply color up front   
    Color.xyz *= Color.w;

    real32 XAxisLength = game_math::length(XAxis);
    real32 YAxisLength = game_math::length(YAxis);

    vec2 NxAxis = (YAxisLength / XAxisLength) * XAxis;
    vec2 NyAxis = (XAxisLength / YAxisLength) * YAxis;

    // NOTE(casey): NzScale could be a parameter if we want people to
    // have control over the amount of scaling in the Z direction
    // that the normals appear to have.
    real32 NzScale = 0.5f * (XAxisLength + YAxisLength);

    real32 InvXAxisLengthSq = 1.0f / game_math::lengthSq(XAxis);
    real32 InvYAxisLengthSq = 1.0f / game_math::lengthSq(YAxis);

    GfxRect FillRectF;
    FillRectF.minPoint = { 1e16, 1e16 };
    FillRectF.maxPoint = { -1e16, -1e16 };

    vec2 P[4] = { Origin, Origin + XAxis, Origin + XAxis + YAxis, Origin + YAxis };
    for (int PIndex = 0;
        PIndex < ArrayCount(P);
        ++PIndex)
    {
        vec2 TestP = P[PIndex];
        int FloorX = game_math::floorToInt32(TestP.x);
        int CeilX = game_math::ceilToInt32(TestP.x) + 1;
        int FloorY = game_math::floorToInt32(TestP.y);
        int CeilY = game_math::ceilToInt32(TestP.y) + 1;

        if (FillRectF.minPoint.x > FloorX) { FillRectF.minPoint.x = FloorX; }
        if (FillRectF.minPoint.y > FloorY) { FillRectF.minPoint.y = FloorY; }
        if (FillRectF.maxPoint.x < CeilX) { FillRectF.maxPoint.x = CeilX; }
        if (FillRectF.maxPoint.y < CeilY) { FillRectF.maxPoint.y = CeilY; }
    }

    //    rectangle2i ClipRect = {0, 0, WidthMax, HeightMax};
    //    rectangle2i ClipRect = {128, 128, 256, 256};
    FillRectF = ClipRect & FillRectF;
    Rect FillRect;
    FillRect.minPoint.x = game_math::floorToInt32(FillRectF.minPoint.x);
    FillRect.minPoint.y = game_math::floorToInt32(FillRectF.minPoint.y);
    FillRect.maxPoint.x = game_math::floorToInt32(FillRectF.maxPoint.x);
    FillRect.maxPoint.y = game_math::floorToInt32(FillRectF.maxPoint.y);

    if (!Even == (((int32)FillRect.minPoint.y) & 1))
    {
        FillRect.minPoint.y += 1.f;
    }

    if (game_rect::hasArea(FillRectF))
    {
        __m128i StartClipMask = _mm_set1_epi8(-1);
        __m128i EndClipMask = _mm_set1_epi8(-1);

        __m128i StartClipMasks[] =
        {
            _mm_slli_si128(StartClipMask, 0 * 4),
            _mm_slli_si128(StartClipMask, 1 * 4),
            _mm_slli_si128(StartClipMask, 2 * 4),
            _mm_slli_si128(StartClipMask, 3 * 4),
        };

        __m128i EndClipMasks[] =
        {
            _mm_srli_si128(EndClipMask, 0 * 4),
            _mm_srli_si128(EndClipMask, 3 * 4),
            _mm_srli_si128(EndClipMask, 2 * 4),
            _mm_srli_si128(EndClipMask, 1 * 4),
        };

        if (FillRect.minPoint.x & 3)
        {
            StartClipMask = StartClipMasks[FillRect.minPoint.x & 3];
            FillRect.minPoint.x = FillRect.minPoint.x & ~3;
        }

        if (FillRect.maxPoint.x & 3)
        {
            EndClipMask = EndClipMasks[FillRect.maxPoint.x & 3];
            FillRect.maxPoint.x = (FillRect.maxPoint.x & ~3) + 4;
        }

        vec2 nXAxis = InvXAxisLengthSq * XAxis;
        vec2 nYAxis = InvYAxisLengthSq * YAxis;

        real32 Inv255 = 1.0f / 255.0f;
        __m128 Inv255_4x = _mm_set1_ps(Inv255);
        real32 One255 = 255.0f;

        __m128 One = _mm_set1_ps(1.0f);
        __m128 Half = _mm_set1_ps(0.5f);
        __m128 Four_4x = _mm_set1_ps(4.0f);
        __m128 One255_4x = _mm_set1_ps(255.0f);
        __m128 Zero = _mm_set1_ps(0.0f);
        __m128i MaskFF = _mm_set1_epi32(0xFF);
        __m128i MaskFFFF = _mm_set1_epi32(0xFFFF);
        __m128i MaskFF00FF = _mm_set1_epi32(0x00FF00FF);
        __m128 Colorr_4x = _mm_set1_ps(Color.x);
        __m128 Colorg_4x = _mm_set1_ps(Color.y);
        __m128 Colorb_4x = _mm_set1_ps(Color.z);
        __m128 Colora_4x = _mm_set1_ps(Color.w);
        __m128 nXAxisx_4x = _mm_set1_ps(nXAxis.x);
        __m128 nXAxisy_4x = _mm_set1_ps(nXAxis.y);
        __m128 nYAxisx_4x = _mm_set1_ps(nYAxis.x);
        __m128 nYAxisy_4x = _mm_set1_ps(nYAxis.y);
        __m128 Originx_4x = _mm_set1_ps(Origin.x);
        __m128 Originy_4x = _mm_set1_ps(Origin.y);
        __m128 MaxColorValue = _mm_set1_ps(255.0f * 255.0f);
        __m128i TexturePitch_4x = _mm_set1_epi32(Texture->imageStats.pitch);

        __m128 WidthM2 = _mm_set1_ps((real32)(Texture->imageStats.width - 2));
        __m128 HeightM2 = _mm_set1_ps((real32)(Texture->imageStats.height - 2));

        uint8* Row = ((uint8*)Buffer->buffer +
            FillRect.minPoint.x * Buffer->imageStats.bytesPerPixel +
            FillRect.minPoint.y * Buffer->imageStats.pitch);
        int32 RowAdvance = 2 * Buffer->imageStats.pitch;

        void* TextureMemory = Texture->data;
        int32 TexturePitch = Texture->imageStats.pitch;

        int minPoint_y = FillRect.minPoint.y;
        int maxPoint_y = FillRect.maxPoint.y;
        int minPoint_x = FillRect.minPoint.x;
        int maxPoint_x = FillRect.maxPoint.x;
        BEGIN_TIMED_BLOCK(ProcessPixel);
        for (int Y = minPoint_y;
            Y < maxPoint_y;
            Y += 2)
        {
            __m128 PixelPy = _mm_set1_ps((real32)Y);
            PixelPy = _mm_sub_ps(PixelPy, Originy_4x);
            __m128 PynX = _mm_mul_ps(PixelPy, nXAxisy_4x);
            __m128 PynY = _mm_mul_ps(PixelPy, nYAxisy_4x);

            __m128 PixelPx = _mm_set_ps((real32)(minPoint_x + 3),
                (real32)(minPoint_x + 2),
                (real32)(minPoint_x + 1),
                (real32)(minPoint_x + 0));
            PixelPx = _mm_sub_ps(PixelPx, Originx_4x);

            __m128i ClipMask = StartClipMask;

            uint32* Pixel = (uint32*)Row;
            for (int XI = minPoint_x;
                XI < maxPoint_x;
                XI += 4)
            {
#define mmSquare(a) _mm_mul_ps(a, a)    
#define M(a, i) ((float *)&(a))[i]
#define Mi(a, i) ((uint32 *)&(a))[i]

                __m128 U = _mm_add_ps(_mm_mul_ps(PixelPx, nXAxisx_4x), PynX);
                __m128 V = _mm_add_ps(_mm_mul_ps(PixelPx, nYAxisx_4x), PynY);

                __m128i WriteMask = _mm_castps_si128(_mm_and_ps(_mm_and_ps(_mm_cmpge_ps(U, Zero),
                    _mm_cmple_ps(U, One)),
                    _mm_and_ps(_mm_cmpge_ps(V, Zero),
                        _mm_cmple_ps(V, One))));
                WriteMask = _mm_and_si128(WriteMask, ClipMask);

                // TODO(casey): Later, re-check if this helps
                //            if(_mm_movemask_epi8(WriteMask))
                {
                    __m128i OriginalDest = _mm_load_si128((__m128i*)Pixel);

                    U = _mm_min_ps(_mm_max_ps(U, Zero), One);
                    V = _mm_min_ps(_mm_max_ps(V, Zero), One);

                    // NOTE(casey): Bias texture coordinates to start
                    // on the boundary between the 0,0 and 1,1 pixels.
                    __m128 tX = _mm_add_ps(_mm_mul_ps(U, WidthM2), Half);
                    __m128 tY = _mm_add_ps(_mm_mul_ps(V, HeightM2), Half);

                    __m128i FetchX_4x = _mm_cvttps_epi32(tX);
                    __m128i FetchY_4x = _mm_cvttps_epi32(tY);

                    __m128 fX = _mm_sub_ps(tX, _mm_cvtepi32_ps(FetchX_4x));
                    __m128 fY = _mm_sub_ps(tY, _mm_cvtepi32_ps(FetchY_4x));

                    FetchX_4x = _mm_slli_epi32(FetchX_4x, 2);
                    FetchY_4x = _mm_or_si128(_mm_mullo_epi16(FetchY_4x, TexturePitch_4x),
                        _mm_slli_epi32(_mm_mulhi_epi16(FetchY_4x, TexturePitch_4x), 16));
                    __m128i Fetch_4x = _mm_add_epi32(FetchX_4x, FetchY_4x);

                    int32 Fetch0 = Mi(Fetch_4x, 0);
                    int32 Fetch1 = Mi(Fetch_4x, 1);
                    int32 Fetch2 = Mi(Fetch_4x, 2);
                    int32 Fetch3 = Mi(Fetch_4x, 3);

                    uint8* TexelPtr0 = ((uint8*)TextureMemory) + Fetch0;
                    uint8* TexelPtr1 = ((uint8*)TextureMemory) + Fetch1;
                    uint8* TexelPtr2 = ((uint8*)TextureMemory) + Fetch2;
                    uint8* TexelPtr3 = ((uint8*)TextureMemory) + Fetch3;

                    __m128i SampleA = _mm_setr_epi32(*(uint32*)(TexelPtr0),
                        *(uint32*)(TexelPtr1),
                        *(uint32*)(TexelPtr2),
                        *(uint32*)(TexelPtr3));

                    __m128i SampleB = _mm_setr_epi32(*(uint32*)(TexelPtr0 + sizeof(uint32)),
                        *(uint32*)(TexelPtr1 + sizeof(uint32)),
                        *(uint32*)(TexelPtr2 + sizeof(uint32)),
                        *(uint32*)(TexelPtr3 + sizeof(uint32)));

                    __m128i SampleC = _mm_setr_epi32(*(uint32*)(TexelPtr0 + TexturePitch),
                        *(uint32*)(TexelPtr1 + TexturePitch),
                        *(uint32*)(TexelPtr2 + TexturePitch),
                        *(uint32*)(TexelPtr3 + TexturePitch));

                    __m128i SampleD = _mm_setr_epi32(*(uint32*)(TexelPtr0 + TexturePitch + sizeof(uint32)),
                        *(uint32*)(TexelPtr1 + TexturePitch + sizeof(uint32)),
                        *(uint32*)(TexelPtr2 + TexturePitch + sizeof(uint32)),
                        *(uint32*)(TexelPtr3 + TexturePitch + sizeof(uint32)));

                    // NOTE(casey): Unpack bilinear samples
                    __m128i TexelArb = _mm_and_si128(SampleA, MaskFF00FF);
                    __m128i TexelAag = _mm_and_si128(_mm_srli_epi32(SampleA, 8), MaskFF00FF);
                    TexelArb = _mm_mullo_epi16(TexelArb, TexelArb);
                    __m128 TexelAa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelAag, 16));
                    TexelAag = _mm_mullo_epi16(TexelAag, TexelAag);

                    __m128i TexelBrb = _mm_and_si128(SampleB, MaskFF00FF);
                    __m128i TexelBag = _mm_and_si128(_mm_srli_epi32(SampleB, 8), MaskFF00FF);
                    TexelBrb = _mm_mullo_epi16(TexelBrb, TexelBrb);
                    __m128 TexelBa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelBag, 16));
                    TexelBag = _mm_mullo_epi16(TexelBag, TexelBag);

                    __m128i TexelCrb = _mm_and_si128(SampleC, MaskFF00FF);
                    __m128i TexelCag = _mm_and_si128(_mm_srli_epi32(SampleC, 8), MaskFF00FF);
                    TexelCrb = _mm_mullo_epi16(TexelCrb, TexelCrb);
                    __m128 TexelCa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelCag, 16));
                    TexelCag = _mm_mullo_epi16(TexelCag, TexelCag);

                    __m128i TexelDrb = _mm_and_si128(SampleD, MaskFF00FF);
                    __m128i TexelDag = _mm_and_si128(_mm_srli_epi32(SampleD, 8), MaskFF00FF);
                    TexelDrb = _mm_mullo_epi16(TexelDrb, TexelDrb);
                    __m128 TexelDa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelDag, 16));
                    TexelDag = _mm_mullo_epi16(TexelDag, TexelDag);

                    // NOTE(casey): Load destination
                    __m128 Destb = _mm_cvtepi32_ps(_mm_and_si128(OriginalDest, MaskFF));
                    __m128 Destg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 8), MaskFF));
                    __m128 Destr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 16), MaskFF));
                    __m128 Desta = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 24), MaskFF));

                    // NOTE(casey): Convert texture from 0-255 sRGB to "linear" 0-1 brightness space
                    __m128 TexelAr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelArb, 16));
                    __m128 TexelAg = _mm_cvtepi32_ps(_mm_and_si128(TexelAag, MaskFFFF));
                    __m128 TexelAb = _mm_cvtepi32_ps(_mm_and_si128(TexelArb, MaskFFFF));

                    __m128 TexelBr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelBrb, 16));
                    __m128 TexelBg = _mm_cvtepi32_ps(_mm_and_si128(TexelBag, MaskFFFF));
                    __m128 TexelBb = _mm_cvtepi32_ps(_mm_and_si128(TexelBrb, MaskFFFF));

                    __m128 TexelCr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelCrb, 16));
                    __m128 TexelCg = _mm_cvtepi32_ps(_mm_and_si128(TexelCag, MaskFFFF));
                    __m128 TexelCb = _mm_cvtepi32_ps(_mm_and_si128(TexelCrb, MaskFFFF));

                    __m128 TexelDr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelDrb, 16));
                    __m128 TexelDg = _mm_cvtepi32_ps(_mm_and_si128(TexelDag, MaskFFFF));
                    __m128 TexelDb = _mm_cvtepi32_ps(_mm_and_si128(TexelDrb, MaskFFFF));

                    // NOTE(casey): Bilinear texture blend
                    __m128 ifX = _mm_sub_ps(One, fX);
                    __m128 ifY = _mm_sub_ps(One, fY);

                    __m128 l0 = _mm_mul_ps(ifY, ifX);
                    __m128 l1 = _mm_mul_ps(ifY, fX);
                    __m128 l2 = _mm_mul_ps(fY, ifX);
                    __m128 l3 = _mm_mul_ps(fY, fX);

                    __m128 Texelr = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAr), _mm_mul_ps(l1, TexelBr)),
                        _mm_add_ps(_mm_mul_ps(l2, TexelCr), _mm_mul_ps(l3, TexelDr)));
                    __m128 Texelg = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAg), _mm_mul_ps(l1, TexelBg)),
                        _mm_add_ps(_mm_mul_ps(l2, TexelCg), _mm_mul_ps(l3, TexelDg)));
                    __m128 Texelb = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAb), _mm_mul_ps(l1, TexelBb)),
                        _mm_add_ps(_mm_mul_ps(l2, TexelCb), _mm_mul_ps(l3, TexelDb)));
                    __m128 Texela = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAa), _mm_mul_ps(l1, TexelBa)),
                        _mm_add_ps(_mm_mul_ps(l2, TexelCa), _mm_mul_ps(l3, TexelDa)));

                    // NOTE(casey): Modulate by incoming color
                    Texelr = _mm_mul_ps(Texelr, Colorr_4x);
                    Texelg = _mm_mul_ps(Texelg, Colorg_4x);
                    Texelb = _mm_mul_ps(Texelb, Colorb_4x);
                    Texela = _mm_mul_ps(Texela, Colora_4x);

                    Texelr = _mm_min_ps(_mm_max_ps(Texelr, Zero), MaxColorValue);
                    Texelg = _mm_min_ps(_mm_max_ps(Texelg, Zero), MaxColorValue);
                    Texelb = _mm_min_ps(_mm_max_ps(Texelb, Zero), MaxColorValue);

                    // NOTE(casey): Go from sRGB to "linear" brightness space
                    Destr = mmSquare(Destr);
                    Destg = mmSquare(Destg);
                    Destb = mmSquare(Destb);

                    // NOTE(casey): Destination blend
                    __m128 InvTexelA = _mm_sub_ps(One, _mm_mul_ps(Inv255_4x, Texela));
                    __m128 Blendedr = _mm_add_ps(_mm_mul_ps(InvTexelA, Destr), Texelr);
                    __m128 Blendedg = _mm_add_ps(_mm_mul_ps(InvTexelA, Destg), Texelg);
                    __m128 Blendedb = _mm_add_ps(_mm_mul_ps(InvTexelA, Destb), Texelb);
                    __m128 Blendeda = _mm_add_ps(_mm_mul_ps(InvTexelA, Desta), Texela);

                    // NOTE(casey): Go from "linear" 0-1 brightness space to sRGB 0-255
#if 1
                    Blendedr = _mm_mul_ps(Blendedr, _mm_rsqrt_ps(Blendedr));
                    Blendedg = _mm_mul_ps(Blendedg, _mm_rsqrt_ps(Blendedg));
                    Blendedb = _mm_mul_ps(Blendedb, _mm_rsqrt_ps(Blendedb));
#else
                    Blendedr = _mm_sqrt_ps(Blendedr);
                    Blendedg = _mm_sqrt_ps(Blendedg);
                    Blendedb = _mm_sqrt_ps(Blendedb);
#endif
                    Blendeda = Blendeda;

                    __m128i Intr = _mm_cvtps_epi32(Blendedr);
                    __m128i Intg = _mm_cvtps_epi32(Blendedg);
                    __m128i Intb = _mm_cvtps_epi32(Blendedb);
                    __m128i Inta = _mm_cvtps_epi32(Blendeda);

                    __m128i Sr = _mm_slli_epi32(Intr, 16);
                    __m128i Sg = _mm_slli_epi32(Intg, 8);
                    __m128i Sb = Intb;
                    __m128i Sa = _mm_slli_epi32(Inta, 24);

                    __m128i Out = _mm_or_si128(_mm_or_si128(Sr, Sg), _mm_or_si128(Sb, Sa));

                    __m128i MaskedOut = _mm_or_si128(_mm_and_si128(WriteMask, Out),
                        _mm_andnot_si128(WriteMask, OriginalDest));
                    _mm_store_si128((__m128i*)Pixel, MaskedOut);
                }

                PixelPx = _mm_add_ps(PixelPx, Four_4x);
                Pixel += 4;

                if ((XI + 8) < maxPoint_x)
                {
                    ClipMask = _mm_set1_epi8(-1);
                }
                else
                {
                    ClipMask = EndClipMask;
                }
            }

            Row += RowAdvance;
        }
    }
}

internal void gdiDrawBitmap(game_render_engine::GfxOffscreenBuffer *destOffscreen, 
                            vec2 Origin, vec2 XAxis, vec2 YAxis, GfxColor Color, game_render_engine::GfxTexture *srcTexture, 
                            const vec4i& clip) {
    BEGIN_TIMED_BLOCK(DrawBitmapLines);

    if (NULL == srcTexture) {
        return;
    }
    if (NULL == srcTexture->data) {
        return;
    }

    GfxRect clipRect = { {clip.x, clip.y},{clip.z, clip.w} };
    DrawRectangleQuickly(destOffscreen, Origin, XAxis, YAxis, Color.rgba, srcTexture, clipRect, true32);
    DrawRectangleQuickly(destOffscreen, Origin, XAxis, YAxis, Color.rgba, srcTexture, clipRect, false32);

    END_TIMED_BLOCK(DrawBitmapLines);
}

inline void
setBufferColor(void* dst, uint32 value, memory_int length) {
    __m128i write = _mm_set1_epi32(value);
    uint32* dest = (uint32*)dst;
    for (uint32 i = 0; i < length; i += 16) {
        _mm_storeu_si128((__m128i*)dest, write);
        dest += 4;
    }
}

/////////////////////////////////////////////////////////////////////////////////


namespace game_render_engine {

    struct gfx_Brush {
        COLORREF cf;
        HBRUSH   brush;
    };
    uint32 colorHashKey(uint32 k) {
        uint32 R = (k)& 0xff;
        uint32 G = (k >> 8) & 0xff;
        uint32 B = (k >> 16) & 0xff;
        uint32 A = (k >> 24) & 0xff;

        return ((R + B + G) ^ A);
    }

    struct gfx_TextRender {
        vec3
            position;
        vec3
            size;
        COLORREF
            brushId;
    };
    /******************************************************************
    * Game subsystems resources
    */
    struct GameRenderTarget {
        HWND
            m_hWnd;
        bool32
            initialized;

        RenderTargetBuffers
            targetBuffers;
        GfxOffscreenBuffer
            *offscreen_buffer;

        RenderGroup
            *renderGroup;
    };

    bool32 CreateGlobalResources(GameState* gameState) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        Assert(targetPtr);

        if (!targetPtr->initialized) {
            HDC hDC = GetDC(targetPtr->m_hWnd);

            PIXELFORMATDESCRIPTOR pfd;
            setBytes8(&pfd, 0, sizeof(pfd));
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_GENERIC_ACCELERATED;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 16;
            pfd.iLayerType = PFD_MAIN_PLANE;
            if (!SetPixelFormat(hDC, 201, &pfd)) {
                int iFormat = ChoosePixelFormat(hDC, &pfd);
                SetPixelFormat(hDC, iFormat, &pfd);
            }
            ReleaseDC(targetPtr->m_hWnd, hDC);
            targetPtr->initialized = true32;
        }

        return (true32);
    }
    bool32 CreateDeviceResources(GameState* gameState) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        Assert(targetPtr);

        if (!CreateGlobalResources(gameState)) {
            return false32;
        }

        RECT rcBounds;
        GetClientRect(targetPtr->m_hWnd, &rcBounds);

        if (!targetPtr->offscreen_buffer->buffer) {
            resizeRenderer(gameState, rcBounds.right - rcBounds.left, rcBounds.bottom - rcBounds.top);
        }

        return true32;
    }

    /******************************************************************
    * Game subsystems resources
    */
    void freeRenderTarget(GameState* gameState) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        Assert(targetPtr);

        targetPtr->offscreen_buffer->buffer = NULL;
    }

    bool32 initRenderer(GameState* gameState, PlatformWindow* window) {
        if (!gameState) {
            return false32;
        }
        if (!window) {
            return false32;
        }
        if (!gameState->renderTargetPtr) {
            gameState->renderTargetPtr = PushStruct(&gameState->engineArena, GameRenderTarget);
        }

        GameRenderTarget*& targetPtr = gameState->renderTargetPtr;
        targetPtr->initialized = false32;
        targetPtr->offscreen_buffer = &window->offscreen_buffer;
        initTargetBuffers(gameState, &targetPtr->targetBuffers);

        if (!createWindow(1200, 683, RC_OPENGL_CLASS_NAME, *window, WS_OVERLAPPEDWINDOW | WS_VISIBLE)) {
            return false32;
        }

        targetPtr->m_hWnd = window->hWnd;
        if (!CreateDeviceResources(gameState)) {
            return false32;
        }

        WINDOWPLACEMENT placement = {};
        placement.length = sizeof(WINDOWPLACEMENT);
        if (GetWindowPlacement(window->hWnd, &placement)) {
            if (placement.showCmd != SW_SHOW) {
                ShowWindow(window->hWnd, SW_SHOW);
            }
        }
        UpdateWindow(targetPtr->m_hWnd);
        return true32;
    }

    void resizeRenderer(GameState* gameState, uint32 width, uint32 height) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;

        game_memory::resetArena(targetPtr->targetBuffers.renderBufferArena);
        initOffscreenBuffer(&targetPtr->targetBuffers, targetPtr->offscreen_buffer, width, height, 32);
    }

    void addRenderAsset(GameState* gameState, uint32 id) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        Assert(targetPtr);
        
        BitmapID i = GetFirstBitmapID(gameState->assets, (AssetType)id);
        BitmapID lastId = GetLastBitmapID(gameState->assets, (AssetType)id);
        for (; i.Value <= lastId.Value; ++i.Value) {
            if (!GetBitmap(gameState->assets, i)) {
                LoadGameBitmap(gameState, { id });
                break;
            }
        }
    }
    //
    RenderGroup* createRenderGroup(GameState* gameState, game_memory::arena_p arena) {
        game_memory::MemoryBlock mem = game_memory::alloc(&gameState->scratchFrameArena, MByte(48), game_memory::AlignNoClear(4));
        RenderGroup* result = (RenderGroup*)mem.m_pMemory;
        setBytes8(result, 0, sizeof(RenderGroup)); // clear!
        result->MaxPushBufferSize = mem.m_iMemoryLength - sizeof(RenderGroup);
        result->PushBufferBase = mem.m_pMemory + sizeof(RenderGroup);
        result->InsideRender = 1;
        result->renderAlpha = 1.0f;
        result->assets = gameState->assets;
        result->drawTarget = gameState->renderTargetPtr->offscreen_buffer;

        result->transform.distanceAboveTarget = gameState->cameraEntity.distanceAboveTarget;
        result->transform.focalLength = gameState->cameraEntity.focalLength;
        result->transform.unitsToPixels = gameState->unitsToPixels;
        result->transform.Orthographic = false;
        result->transform.scale = 1.0f;

        return (result);
    }

    void perspective(RenderGroup *RenderGroup, int32 PixelWidth, int32 PixelHeight,
        real32 unitsToPixels, real32 focalLength, real32 distanceAboveTarget) {
        // TODO(Roman): Need to adjust this based on buffer size
        real32 PixelsToMeters = game_math::safeRatio1(1.0f, unitsToPixels);

        RenderGroup->MonitorHalfDimInMeters = v2(0.5f*PixelWidth*PixelsToMeters,
                                                 0.5f*PixelHeight*PixelsToMeters);

        RenderGroup->transform.unitsToPixels = unitsToPixels;
        RenderGroup->transform.focalLength = focalLength; // NOTE(Roman): Meters the person is sitting from their monitor
        RenderGroup->transform.distanceAboveTarget = distanceAboveTarget;
        RenderGroup->transform.screenCenter = v2(0.5f*PixelWidth, 0.5f*PixelHeight);

        RenderGroup->transform.Orthographic = false;
    }
    void orthographic(RenderGroup *RenderGroup, int32 PixelWidth, int32 PixelHeight,
        real32 unitsToPixels, real32 focalLength, real32 distanceAboveTarget) {
        real32 PixelsToMeters = game_math::safeRatio1(1.0f, unitsToPixels);
        RenderGroup->MonitorHalfDimInMeters = v2(0.5f*PixelWidth*PixelsToMeters,
                                                 0.5f*PixelHeight*PixelsToMeters);

        RenderGroup->transform.unitsToPixels = unitsToPixels;
        RenderGroup->transform.focalLength = focalLength; // NOTE(Roman): Meters the person is sitting from their monitor
        RenderGroup->transform.distanceAboveTarget = distanceAboveTarget;
        RenderGroup->transform.screenCenter = v2(0.5f*PixelWidth, 0.5f*PixelHeight);

        RenderGroup->transform.Orthographic = true;
    }
    //
    // startRenderProcess : initiates the renderer to start back buffer rendering, grabs any memory it needs to build the render comand line
    RenderGroup* startRenderProcess(GameState* gameState) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        Assert(targetPtr);

        targetPtr->renderGroup = createRenderGroup(gameState, &gameState->scratchFrameArena);
        return targetPtr->renderGroup;
    }
    //
    // - completeRender() will finish up, launch the rendering process and flip buffers on the flip chain.
    bool32 completeRender(GameState* gameState) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        RenderGroup* renderGroup = gameState->renderTargetPtr->renderGroup;
        //
        vec4i clip = { 0, 0, renderGroup->drawTarget->imageStats.width, renderGroup->drawTarget->imageStats.height };

        uint8* command = renderGroup->PushBufferBase;
        uint32 processed = 0;

        BEGIN_TIMED_BLOCK(GameRender);
        RenderGroupEntryHeader* header;
        while (processed < renderGroup->PushBufferSize) {
            BEGIN_TIMED_BLOCK(ProcessCommand);
            processed += sizeof(RenderGroupEntryHeader);
            header = (RenderGroupEntryHeader*)command;
            command += sizeof(RenderGroupEntryHeader);
            switch (header->type) {
                case RenderGroupEntryType_RenderEntryClear: {
                    BEGIN_TIMED_BLOCK(SetBufferValue);
                    processed += sizeof(RenderEntryClear);
                    RenderEntryClear* clearEntry = (RenderEntryClear*)command;
                    // DO something!!
                    setBufferColor(renderGroup->drawTarget->buffer, rgba_pack4x8(clearEntry->color), getBitmapSize(renderGroup->drawTarget->imageStats));
                    // continue;
                    command += sizeof(RenderEntryClear);
                    END_TIMED_BLOCK(SetBufferValue);
                    break;
                }
                case RenderGroupEntryType_RenderEntryBitmap: {
                    BEGIN_TIMED_BLOCK(DrawInterlacedBitmap);
                    processed += sizeof(RenderEntryBitmap);
                    RenderEntryBitmap* drawBitmap = (RenderEntryBitmap*)command;
                    // DO something!!
                    vec2 sizex = v2(1, 0) * drawBitmap->transform.scale;
                    vec2 sizey = v2(0, 1) * drawBitmap->transform.scale;

                    vec2 offset = drawBitmap->transform.p.xy - game_math::hadamard(drawBitmap->bitmap->alignPercentage, v2(sizex.x, sizey.y));
                    offset.y += drawBitmap->transform.p.z;

                    gdiDrawBitmap(renderGroup->drawTarget, offset, sizex, sizey, drawBitmap->color, drawBitmap->bitmap, clip);

                    // continue;
                    command += sizeof(RenderEntryBitmap);
                    END_TIMED_BLOCK(DrawInterlacedBitmap);
                    break;
                }
                case RenderGroupEntryType_RenderEntryRect: {
                    BEGIN_TIMED_BLOCK(DrawRectangle);
                    processed += sizeof(RenderEntryRect);
                    RenderEntryRect* drawRectangle = (RenderEntryRect*)command;
                    GfxRect rc = game_rect::createGfxRect(drawRectangle->transform.p.xy, drawRectangle->dim);
                    game_rect::offset(rc, 0, drawRectangle->transform.p.z);
                    if (drawRectangle->fill) {
                        DrawRectangle(renderGroup->drawTarget, { { rc.minPoint.x, rc.minPoint.y } }, { { rc.maxPoint.x, rc.maxPoint.y } }, drawRectangle->color);
                    } else {
						drawHLine(renderGroup->drawTarget, rc.minPoint.x, rc.minPoint.y, rc.maxPoint.x - rc.minPoint.x, drawRectangle->color);
						drawHLine(renderGroup->drawTarget, rc.minPoint.x, rc.maxPoint.y, rc.maxPoint.x - rc.minPoint.x, drawRectangle->color);
						drawVLine(renderGroup->drawTarget, rc.minPoint.x, rc.minPoint.y, rc.maxPoint.y - rc.minPoint.y, drawRectangle->color);
						drawVLine(renderGroup->drawTarget, rc.maxPoint.x, rc.minPoint.y, rc.maxPoint.y - rc.minPoint.y, drawRectangle->color);
					}
                    // continue;
                    command += sizeof(RenderEntryRect);
                    END_TIMED_BLOCK(DrawRectangle);
                    break;
                }
                default: {
                    processed += header->length;
                    command += header->length;
                    break;
                }
            }
            END_TIMED_BLOCK(ProcessCommand);
        }
        END_TIMED_BLOCK(GameRender);
        return true32;
    }

    void pushClear(GameState* gameState, uint32 color) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        RenderGroup* renderGroup = targetPtr->renderGroup;
        //
        // First find any old clear command and set it to RenderEntryNoOP
        uint8* command = targetPtr->renderGroup->PushBufferBase;
        uint32 processed = 0;
        while (processed < targetPtr->renderGroup->PushBufferSize) {
            processed += sizeof(RenderGroupEntryHeader);
            RenderGroupEntryHeader* header = (RenderGroupEntryHeader*)command;
            if (header->type == RenderGroupEntryType_RenderEntryClear) {
                header->type = RenderGroupEntryType_RenderEntryNoOP;
            }
            command += header->length;
            processed += header->length;
        }

        RenderEntryClear* data = (RenderEntryClear*)pushRenderElement(renderGroup, RenderEntryClear);
        if (NULL != data) {
            data->color = rgba_unpack4x8(color);
        }
    }

    void pushRect(RenderGroup *renderGroup, const vec3& offset, const vec3& rectDim, bool32 fill, const GfxColor color)
    {
		if (renderGroup->renderAlpha > game_math::epsilon) {
			RenderEntryTransform transform = getRenderEntityTransform(&renderGroup->transform, offset);
			if (transform.valid) {
				RenderEntryRect* data = (RenderEntryRect*)pushRenderElement(renderGroup, RenderEntryRect);
				if (NULL != data) {
					data->color.rgba = color.rgba * renderGroup->renderAlpha;
					//data->color.a = 255.0f;
					data->fill = fill;
					data->dim = v2(rectDim.x, rectDim.z) * transform.scale; // NOTE(Roman): sizing should be done here!!!
					//
					data->transform = transform;
				}
			}
		}
    }
    bool32 pushBitmap(RenderGroup *renderGroup, uint32 id, real32 height, const vec3& offset, const GfxColor color) {
		if (renderGroup->renderAlpha > game_math::epsilon) {
			RenderEntryTransform transform = getRenderEntityTransform(&renderGroup->transform, offset);
			if (transform.valid) {
				game_render_engine::GfxTexture *image = GetBitmap(renderGroup->assets, BitmapID{ id });
				if (image) {
					RenderEntryBitmap* bitmap = (RenderEntryBitmap*)pushRenderElement(renderGroup, RenderEntryBitmap);
					if (NULL != bitmap) {
						bitmap->color.rgba = color.rgba * renderGroup->renderAlpha;
						bitmap->transform = transform;
						bitmap->bitmap = image;
					}
				}
				else {
					return false32;
				}
			}
		}
        return true32;
    }
    bool32 pushSprite(RenderGroup *renderGroup, GfxTexture* tex, real32 height, const vec3& offset, const GfxColor color) {
		if (renderGroup->renderAlpha > game_math::epsilon) {
			RenderEntryTransform transform = getRenderEntityTransform(&renderGroup->transform, offset);
			if (transform.valid) {
				if (tex) {
					RenderEntryBitmap* bitmap = (RenderEntryBitmap*)pushRenderElement(renderGroup, RenderEntryBitmap);
					if (NULL != bitmap) {
						bitmap->color.rgba = color.rgba * renderGroup->renderAlpha;
						bitmap->transform = transform;
						bitmap->bitmap = tex;
					}
				}
				else {
					return false32;
				}
			}
		}
        return true32;
    }
}

#endif
#define __GC_2DOgl_RENDER_DEVICE_H_
#endif//__GC_2DOgl_RENDER_DEVICE_H_
