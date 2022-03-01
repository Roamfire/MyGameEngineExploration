
#include "../RaidCore/Headers/RaidCore_RenderHandler.h"

internal void
HandleDebugCycleCounters(debug_timer *timers, real32 d) {
    const char* d_name[] = {
        "debugGameLoop",
        "debugGameUpdate",
        "debugGameRenderInit",
        "debugGameRender",
        "debugLoadSim",
        "debugUpdateLoop",
        "debugProcessVisuals",
        "debugPushRectangle",
        "debugPushBitmap",
        "debugProcessCommand",
        "debugDrawRectangle",
        "debugDrawInterlacedBitmap",
        "debugDrawBitmapLines",
        "debugProcessPixel",
        "debugSetBufferValue",
        "debugRenderBlt",
        "",
        ""
    };
    OutputDebugStringA("DEBUG CYCLE COUNTS:\n");
    char TextBuffer[256];

    _snprintf_s(TextBuffer, sizeof(TextBuffer),
                "  estimate for 2.6GHz FPS: %I64u\n",
                2600000000 / (timers[0].CycleCount / timers[0].HitCount));
    OutputDebugStringA(TextBuffer);

    _snprintf_s(TextBuffer, sizeof(TextBuffer),
                "  average FPS: %I64u\n",
                (uint64)d);
    OutputDebugStringA(TextBuffer);

    for (int CounterIndex = 0;
         CounterIndex < timer_id_count;
         ++CounterIndex) {
        debug_timer *Counter = timers + CounterIndex;

        if (Counter->HitCount) {
            _snprintf_s(TextBuffer, sizeof(TextBuffer),
                        "  %s: %I64ucy %Iuh %I64ucy/h\n",
                        d_name[CounterIndex],
                        Counter->CycleCount,
                        Counter->HitCount,
                        Counter->CycleCount / Counter->HitCount);
            OutputDebugStringA(TextBuffer);
            Counter->HitCount = 0;
            Counter->CycleCount = 0;
        }
    }
}

namespace const_r {
    __m128i MaskFF = _mm_set1_epi32(0xff);
    __m128i MaskFFFF = _mm_set1_epi32(0xFFFF);
    __m128i MaskFF00FF = _mm_set1_epi32(0x00FF00FF);
    __m128 One = _mm_set1_ps(1.0f);
    __m128 Zero = _mm_set1_ps(0.0f);
    __m128 One255_4x = _mm_set1_ps(255.0f);
    __m128 Inv255_4x = _mm_set1_ps(1.0f / 255.0f);
};

#define DBG_DRAWRECT(a,b,c,d) DrawRect(a,b,c,d)
#define DBG_DRAWLINE(a,b,c,d) DrawBox(a,b,c,d)

static void drawHLine(game_render_engine::GfxOffscreenBuffer * dest, const real32 x, const real32 y, const real32 len, GfxColor& color) {
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

    GfxColor Null = rgba((uint32)color.r, (uint32)color.g, (uint32)color.b, 0U);
    GfxColor left, top, right, bottom;
    if (fMinY != MinY) {
        top.rgba = game_math::lerp(color.rgba, y - (real32)fMinY, Null.rgba);
        bottom.rgba = game_math::lerp(Null.rgba, (real32)fMinY + 1.0f - y, color.rgba);
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
static void drawVLine(game_render_engine::GfxOffscreenBuffer * dest, const real32 x, const real32 y, const real32 len, GfxColor& color) {
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

    GfxColor Null = rgba((uint32)color.r, (uint32)color.g, (uint32)color.b, 0U);
    GfxColor left, top, right, bottom;
    if (fMinX != MinX) {
        left.rgba = game_math::lerp(color.rgba, x - (real32)fMinX, Null.rgba);
        right.rgba = game_math::lerp(Null.rgba, (real32)fMinX + 1.0f - x, color.rgba);
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
static void DrawBox(game_render_engine::GfxOffscreenBuffer * destOffscreen, const vec2& vMin, const vec2& vMax, GfxColor& Color) {
    drawVLine(destOffscreen, vMin.x, vMin.y, vMax.y - vMin.y, Color);
    drawVLine(destOffscreen, vMax.x, vMin.y, vMax.y - vMin.y, Color);
    drawHLine(destOffscreen, vMin.x, vMin.y, vMax.x - vMin.x, Color);
    drawHLine(destOffscreen, vMin.x, vMax.y, vMax.x - vMin.x, Color);
}
static void DrawRect(game_render_engine::GfxOffscreenBuffer * destOffscreen, const vec2& vMin, const vec2& vMax, GfxColor& Color) {
    vec2 _min = v2(_Max(0.0f, vMin.x), _Max(0.0f, vMin.y));
    vec2 _max = v2(_Min((real32)destOffscreen->imageStats.width, vMax.x), _Min((real32)destOffscreen->imageStats.height, vMax.y));
    int32 XMin = game_math::floorToInt32(_min.x);
    int32 YMin = game_math::floorToInt32(_min.y);
    int32 XMax = game_math::ceilToInt32(_max.x);
    int32 YMax = game_math::ceilToInt32(_max.y);

    int WidthMax = (destOffscreen->imageStats.width - 1) - 3;
    int HeightMax = (destOffscreen->imageStats.height - 1) - 3;

    if ((XMin < XMax) && (YMin < YMax)) {

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

        if (XMin & 3) {
            StartClipMask = StartClipMasks[XMin & 3];
            XMin = XMin & ~3;
        }

        if (XMax & 3) {
            EndClipMask = EndClipMasks[XMax & 3];
            XMax = (XMax & ~3) + 4;
        }

        vec2 nXAxis = v2(1.f, 0.f);
        vec2 nYAxis = v2(0.f, 1.f);

        GfxColor cl = rgba01(255.0f, 255.0f, 255.0f, 255.0f);
        uint32 iColor = rgba_pack4x8(Color);

        __m128 Colorr_4x = _mm_set1_ps(cl.r);
        __m128 Colorg_4x = _mm_set1_ps(cl.g);
        __m128 Colorb_4x = _mm_set1_ps(cl.b);
        __m128 Colora_4x = _mm_set1_ps(cl.a);
        __m128 nXAxisx_4x = _mm_set1_ps(nXAxis.x);
        __m128 nXAxisy_4x = _mm_set1_ps(nXAxis.y);
        __m128 nYAxisx_4x = _mm_set1_ps(nYAxis.x);
        __m128 nYAxisy_4x = _mm_set1_ps(nYAxis.y);
        __m128 Originx_4x = _mm_set1_ps(_min.x);
        __m128 Originy_4x = _mm_set1_ps(_min.y);
        //
        __m128 Four_4x = _mm_set1_ps(4.0f);

        __m128 WidthM2 = _mm_set1_ps((real32)(XMax - XMin - 2));
        __m128 HeightM2 = _mm_set1_ps((real32)(YMax - YMin - 2));

        const uint32 srcByteWidth = ((XMax - XMin) * destOffscreen->imageStats.bytesPerPixel);
        const uint32 destByteWidth = (destOffscreen->imageStats.width * destOffscreen->imageStats.bytesPerPixel);
        const uint32 destByteAdvance = destByteWidth << 1;

        uint8 *Row = ((uint8*)destOffscreen->buffer +
                      (XMin * destOffscreen->imageStats.bytesPerPixel) +
                      YMin * destByteWidth);

        // NOTE(Roman): Tall textures will overflow, so we break them into lo and hi
        __m128i TexturePitchHi = _mm_set1_epi32((srcByteWidth & 0x0000ff00) >> 8);
        __m128i TexturePitchLo = _mm_set1_epi32(srcByteWidth & 0x000000ff);

        uint32 rMode = _MM_GET_ROUNDING_MODE();
        _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);

        for (int Y = YMin; Y < YMax; Y += 2) {
            uint32 *Pixel = (uint32 *)Row;
            __m128 PixelPy = _mm_set1_ps((real32)Y);
            PixelPy = _mm_sub_ps(PixelPy, Originy_4x);
            __m128 PynX = _mm_mul_ps(PixelPy, nXAxisy_4x);
            __m128 PynY = _mm_mul_ps(PixelPy, nYAxisy_4x);

            __m128i ClipMask = StartClipMask;
            __m128 PixelPx = _mm_set_ps((real32)(XMin + 3),
                                        (real32)(XMin + 2),
                                        (real32)(XMin + 1),
                                        (real32)(XMin + 0));
            PixelPx = _mm_sub_ps(PixelPx, Originx_4x);

            for (int XI = XMin; XI < XMax; XI += 4) {

#define mmSquare(a) _mm_mul_ps(a, a)    
#define M(a, i) ((float *)&(a))[i]
#define Mi(a, i) ((uint32 *)&(a))[i]

                __m128 U = _mm_add_ps(_mm_mul_ps(PixelPx, nXAxisx_4x), PynX);
                __m128 V = _mm_add_ps(_mm_mul_ps(PixelPx, nYAxisx_4x), PynY);

                __m128i WriteMask = _mm_castps_si128(_mm_and_ps(_mm_and_ps(_mm_cmpge_ps(U, const_r::Zero),
                    _mm_cmple_ps(U, const_r::One)),
                    _mm_and_ps(_mm_cmpge_ps(V, const_r::Zero),
                    _mm_cmple_ps(V, const_r::One))));
                WriteMask = _mm_and_si128(WriteMask, ClipMask);

                // clamp U,V to 0::1 range
                U = _mm_min_ps(_mm_max_ps(const_r::Zero, U), const_r::One);
                V = _mm_min_ps(_mm_max_ps(const_r::Zero, V), const_r::One);

                //if (_mm_movemask_epi8(WriteMask)) 
                {
                    __m128i OriginalDest = _mm_load_si128((__m128i *)Pixel);

                    __m128 tX_x4 = _mm_mul_ps(U, WidthM2);
                    __m128 tY_x4 = _mm_mul_ps(V, HeightM2);

                    __m128i FetchX_4x = _mm_cvttps_epi32(tX_x4);
                    __m128i FetchY_4x = _mm_cvttps_epi32(tY_x4);

                    __m128 fX = _mm_sub_ps(tX_x4, _mm_cvtepi32_ps(FetchX_4x));
                    __m128 fY = _mm_sub_ps(tY_x4, _mm_cvtepi32_ps(FetchY_4x));

                    __m128i SampleA = _mm_setr_epi32(iColor,
                                                     iColor,
                                                     iColor,
                                                     iColor);
                    __m128i SampleB = _mm_setr_epi32(iColor,
                                                     iColor,
                                                     iColor,
                                                     iColor);
                    __m128i SampleC = _mm_setr_epi32(iColor,
                                                     iColor,
                                                     iColor,
                                                     iColor);
                    __m128i SampleD = _mm_setr_epi32(iColor,
                                                     iColor,
                                                     iColor,
                                                     iColor);

                    __m128 TexelAr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleA, 16), const_r::MaskFF));
                    __m128 TexelAg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleA, 8), const_r::MaskFF));
                    __m128 TexelAb = _mm_cvtepi32_ps(_mm_and_si128(SampleA, const_r::MaskFF));
                    __m128 TexelAa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleA, 24), const_r::MaskFF));

                    __m128 TexelBr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleB, 16), const_r::MaskFF));
                    __m128 TexelBg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleB, 8), const_r::MaskFF));
                    __m128 TexelBb = _mm_cvtepi32_ps(_mm_and_si128(SampleB, const_r::MaskFF));
                    __m128 TexelBa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleB, 24), const_r::MaskFF));

                    __m128 TexelCr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleC, 16), const_r::MaskFF));
                    __m128 TexelCg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleC, 8), const_r::MaskFF));
                    __m128 TexelCb = _mm_cvtepi32_ps(_mm_and_si128(SampleC, const_r::MaskFF));
                    __m128 TexelCa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleC, 24), const_r::MaskFF));

                    __m128 TexelDr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleD, 16), const_r::MaskFF));
                    __m128 TexelDg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleD, 8), const_r::MaskFF));
                    __m128 TexelDb = _mm_cvtepi32_ps(_mm_and_si128(SampleD, const_r::MaskFF));
                    __m128 TexelDa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleD, 24), const_r::MaskFF));

                    __m128 Destr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 16), const_r::MaskFF));
                    __m128 Destg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 8), const_r::MaskFF));
                    __m128 Destb = _mm_cvtepi32_ps(_mm_and_si128(OriginalDest, const_r::MaskFF));
                    __m128 Desta = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 24), const_r::MaskFF));

                    // NOTE(Roman): Convert texture from 0-255 sRGB to "linear" 0-1 brightness space
                    TexelAr = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelAr));
                    TexelAg = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelAg));
                    TexelAb = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelAb));
                    TexelAa = _mm_mul_ps(const_r::Inv255_4x, TexelAa);

                    TexelBr = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelBr));
                    TexelBg = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelBg));
                    TexelBb = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelBb));
                    TexelBa = _mm_mul_ps(const_r::Inv255_4x, TexelBa);

                    TexelCr = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelCr));
                    TexelCg = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelCg));
                    TexelCb = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelCb));
                    TexelCa = _mm_mul_ps(const_r::Inv255_4x, TexelCa);

                    TexelDr = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelDr));
                    TexelDg = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelDg));
                    TexelDb = mmSquare(_mm_mul_ps(const_r::Inv255_4x, TexelDb));
                    TexelDa = _mm_mul_ps(const_r::Inv255_4x, TexelDa);

                    // NOTE(Roman): Bilinear texture blend
                    __m128 ifX = _mm_sub_ps(const_r::One, fX);
                    __m128 ifY = _mm_sub_ps(const_r::One, fY);

                    __m128 l0 = _mm_mul_ps(ifY, ifX);
                    __m128 l1 = _mm_mul_ps(ifY, fX);
                    __m128 l2 = _mm_mul_ps(fY, ifX);
                    __m128 l3 = _mm_mul_ps(fY, fX);

                    __m128 Texelr = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAr), _mm_mul_ps(l1, TexelBr)), _mm_mul_ps(l2, TexelCr)), _mm_mul_ps(l3, TexelDr));
                    __m128 Texelg = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAg), _mm_mul_ps(l1, TexelBg)), _mm_mul_ps(l2, TexelCg)), _mm_mul_ps(l3, TexelDg));
                    __m128 Texelb = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAb), _mm_mul_ps(l1, TexelBb)), _mm_mul_ps(l2, TexelCb)), _mm_mul_ps(l3, TexelDb));
                    __m128 Texela = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAa), _mm_mul_ps(l1, TexelBa)), _mm_mul_ps(l2, TexelCa)), _mm_mul_ps(l3, TexelDa));

                    // NOTE(Roman): Modulate by incoming color
                    Texelr = _mm_mul_ps(Texelr, Colorr_4x);
                    Texelg = _mm_mul_ps(Texelg, Colorg_4x);
                    Texelb = _mm_mul_ps(Texelb, Colorb_4x);
                    Texela = _mm_mul_ps(Texela, Colora_4x);

                    Texelr = _mm_min_ps(_mm_max_ps(Texelr, const_r::Zero), const_r::One);
                    Texelg = _mm_min_ps(_mm_max_ps(Texelg, const_r::Zero), const_r::One);
                    Texelb = _mm_min_ps(_mm_max_ps(Texelb, const_r::Zero), const_r::One);

                    // NOTE(Roman): Go from sRGB to "linear" brightness space
                    Destr = mmSquare(_mm_mul_ps(const_r::Inv255_4x, Destr));
                    Destg = mmSquare(_mm_mul_ps(const_r::Inv255_4x, Destg));
                    Destb = mmSquare(_mm_mul_ps(const_r::Inv255_4x, Destb));
                    Desta = _mm_mul_ps(const_r::Inv255_4x, Desta);

                    // NOTE(Roman): Destination blend
                    __m128 InvTexelA = _mm_sub_ps(const_r::One, Texela);
                    __m128 Blendedr = _mm_add_ps(_mm_mul_ps(InvTexelA, Destr), Texelr);
                    __m128 Blendedg = _mm_add_ps(_mm_mul_ps(InvTexelA, Destg), Texelg);
                    __m128 Blendedb = _mm_add_ps(_mm_mul_ps(InvTexelA, Destb), Texelb);
                    __m128 Blendeda = _mm_add_ps(_mm_mul_ps(InvTexelA, Desta), Texela);

                    // NOTE(Roman): Go from "linear" 0-1 brightness space to sRGB 0-255
                    Blendedr = _mm_mul_ps(const_r::One255_4x, _mm_sqrt_ps(Blendedr));
                    Blendedg = _mm_mul_ps(const_r::One255_4x, _mm_sqrt_ps(Blendedg));
                    Blendedb = _mm_mul_ps(const_r::One255_4x, _mm_sqrt_ps(Blendedb));
                    Blendeda = _mm_mul_ps(const_r::One255_4x, Blendeda);

                    __m128i Intr = _mm_cvtps_epi32(Blendedr);
                    __m128i Intg = _mm_cvtps_epi32(Blendedg);
                    __m128i Intb = _mm_cvtps_epi32(Blendedb);
                    __m128i Inta = _mm_cvtps_epi32(Blendeda);

                    __m128i out = _mm_and_si128(WriteMask, _mm_or_si128(Intb, _mm_or_si128(_mm_slli_epi32(Intr, 16), _mm_or_si128(_mm_slli_epi32(Intg, 8), _mm_slli_epi32(Inta, 24)))));
                    __m128i pixelIn = _mm_or_si128(out, _mm_andnot_si128(WriteMask, OriginalDest));

                    _mm_store_si128((__m128i*)Pixel, pixelIn);
                }
                Pixel += 4;
                PixelPx = _mm_add_ps(PixelPx, Four_4x);
                if ((XI + 8) < XMax) {
                    ClipMask = _mm_set1_epi8(-1);
                } else {
                    ClipMask = EndClipMask;
                }
            }
            Row += destByteAdvance;
        }
        _MM_SET_ROUNDING_MODE(rMode);
    }
}

struct dbgFrameData {
    real32 delta;
    real32 frameTime;
    dbgFrameData* n;
};

static dbgFrameData * dbgFrameRoot = 0;
static dbgFrameData * dbgFrameCurrent = 0;
static real32 frame = 0.0f;

void 
Win32DebugSetup(uint32 max) {
    if (NULL != dbgFrameRoot) {
        memory_func::Free(dbgFrameRoot);
    }
    dbgFrameCurrent = dbgFrameRoot = memory_func::ViAllocate<dbgFrameData>(max);
    for(int i=0; i < max; ++i) {
        (dbgFrameRoot + i)->delta = 0.0f;
        (dbgFrameRoot + i)->n = dbgFrameRoot + i + 1;
    }
    dbgFrameRoot[max - 1].n = dbgFrameRoot;
}

void
Win32DebugSyncSet(uint32 playPtr, uint32 maxPlay) {
    dbgFrameCurrent->delta = (real32)playPtr / (real32)maxPlay;
}
void 
Win32DebugFrameSet(real32 frameTimeDelta) {
    frame += frameTimeDelta;
    dbgFrameCurrent->frameTime = (real32)((uint32)(frame * 1000.0f) % 1000) / 1000.0f;
}

void
Win32DebugSyncDisplay(game_render_engine::GfxOffscreenBuffer* offscreen) {
    DrawBox(offscreen, v2(32.f, 10.f), v2(offscreen->imageStats.width - 32.f, 70.f), rgba(0, 255, 255));
    for (uint32 i = 1; i < 30; ++i) {
        real32 ratio = (real32)i / 30.0f;
        real32 x = ((offscreen->imageStats.width - 64) * ratio) + 32.f;
        drawVLine(offscreen, x, 51.f, 19.f, rgba(255, 0, 255));
    }
    //
    {
        dbgFrameCurrent = dbgFrameCurrent->n;
        dbgFrameData * dbgF = dbgFrameCurrent;

        real32 x;
        while (dbgF->n!=dbgFrameCurrent) {
            x = ((offscreen->imageStats.width - 64) * dbgF->delta) + 32.f;
            drawVLine(offscreen, x, 11.f, 20.f, rgba(255, 255, 255));
            x = ((offscreen->imageStats.width - 64) * dbgF->frameTime) + 32.f;
            drawVLine(offscreen, x, 30.f, 20.f, rgba(255, 255, 0));
            dbgF = dbgF->n;
        } 
    }
}

internal
uint32 command[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
internal
uint32 commandIndex = 0;

void
Win32HandleKeyException(uint32 VKCode, bool32 IsDown, user_input::GameInput* gameInput) {
    user_input::IOActionState act = gameInput->m_keys[user_input::key_btn_special0];
    if (VKCode >= 'a' && VKCode <= 'z') {

    }
}
