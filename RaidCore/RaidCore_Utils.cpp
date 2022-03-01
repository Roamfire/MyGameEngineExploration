/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#include "../RaidCore/Headers/RaidCore_Common.h"
#include "../RaidCore/Headers/RaidCore_UtilsHeader.h"
#include "../RaidCore/Headers/RaidCore_ImageUtils.h"
#include "../RaidCore/Headers/RaidCore_RenderHandler.h"

game_memory::MemoryBlock getBitmapRaw(uint8 * data, game_memory::arena_p pArena) {
    tagBitmap& header = *(tagBitmap*)data;

    //
    // line_size should be 4byte aligned, so we add 31bits to the total count of bits per line,
    // then we bit shift to the right 5 times (/32) and shift the result to the left 2 times (*4)
    // that would be the total byte aligned count we need per line.
    int32 line_size = ((header.biBitCount * header.biWidth + 31) >> 5) << 2;
    uint32 memory_size = line_size * game_math::absolute(header.biHeight);

    Assert(pArena);

    game_memory::MemoryBlock bmpMem = game_memory::alloc(pArena, game_memory::GetAlignedSize(sizeof(game_render_engine::GfxTexture), 16) + memory_size);
    game_render_engine::GfxTexture * bitmapPtr = (game_render_engine::GfxTexture*)bmpMem.m_pMemory;
    bitmapPtr->data = bmpMem.m_pMemory + game_memory::GetAlignedSize(sizeof(game_render_engine::GfxTexture), 16);
    //
    bitmapPtr->type = game_render_engine::textureType_inMemory;
    bitmapPtr->imageStats.width = header.biWidth;
    bitmapPtr->imageStats.height = game_math::absolute(header.biHeight);
    bitmapPtr->imageStats.bytesPerPixel = header.biBitCount / 8;
    bitmapPtr->imageStats.pitch = (uint32)header.biWidth * bitmapPtr->imageStats.bytesPerPixel; // safeCast2U8(line_size - (uint32)header.biWidth * bitmapPtr->imageStats.bytesPerPixel);
    //
    bitmapPtr->widthOverHeight = game_math::safeRatio0(bitmapPtr->imageStats.width, bitmapPtr->imageStats.height);
    //
    uint32 RedMask = header.redMask;
    uint32 GreenMask = header.greenMask;
    uint32 BlueMask = header.blueMask;
    uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
    bit_scan_result RedScan = findLeastSignificantSetBit(RedMask);
    bit_scan_result GreenScan = findLeastSignificantSetBit(GreenMask);
    bit_scan_result BlueScan = findLeastSignificantSetBit(BlueMask);
    bit_scan_result AlphaScan = findLeastSignificantSetBit(AlphaMask);
    //
    Assert(RedScan.Found);
    Assert(GreenScan.Found);
    Assert(BlueScan.Found);
    Assert(AlphaScan.Found);
    //
    // Set the RGBA format in loaded image data
    int32 RedShiftDown = (int32)RedScan.Index;
    int32 GreenShiftDown = (int32)GreenScan.Index;
    int32 BlueShiftDown = (int32)BlueScan.Index;
    int32 AlphaShiftDown = (int32)AlphaScan.Index;

    uint8* line_pos = 0;
    if (header.biHeight > 0) {
        line_pos = data + header.bfOffset;
    } else {
        line_pos = data + header.bfOffset + ((bitmapPtr->imageStats.height - 1) * line_size);
        line_size = -line_size;
    }
    //
    // Copy image data
    for (int32 i = 0; i < bitmapPtr->imageStats.height; ++i) {
        uint8* dst = bitmapPtr->data + i * game_math::absolute(line_size);

        for (int32 x = 0; x < bitmapPtr->imageStats.width; ++x) {
            uint32 C = *(uint32*)(line_pos + (x<<2));
            GfxColor Texel = { (real32)((C & RedMask) >> RedShiftDown),
                (real32)((C & GreenMask) >> GreenShiftDown),
                (real32)((C & BlueMask) >> BlueShiftDown),
                (real32)((C & AlphaMask) >> AlphaShiftDown) };

            real32 Inv255 = 1.0f / 255.0f;

            Texel.a *= Inv255;
            Texel.r = game_math::square(Inv255*Texel.r)*Texel.a;
            Texel.g = game_math::square(Inv255*Texel.g)*Texel.a;
            Texel.b = game_math::square(Inv255*Texel.b)*Texel.a;
            real32 One255 = 255.0f;

            Texel.a *= One255;
            Texel.r = One255*game_math::squareRoot(Texel.r);
            Texel.g = One255*game_math::squareRoot(Texel.g);
            Texel.b = One255*game_math::squareRoot(Texel.b);

            *((uint32*)dst) = (((uint32)(Texel.a + 0.5f) << 24) |
                             ((uint32)(Texel.r + 0.5f) << 16) |
                             ((uint32)(Texel.g + 0.5f) << 8) |
                             ((uint32)(Texel.b + 0.5f) << 0));
            dst += 4;
        }
        line_pos += line_size;
    }
    return(bmpMem);
}

game_memory::MemoryBlock copyBitmap(game_render_engine::GfxTexture* data, game_memory::arena_p pArena) {
    memory_int hdr_size = game_memory::GetAlignedSize(sizeof(game_render_engine::GfxTexture), 16);
    memory_int data_size = game_memory::GetAlignedSize(game_render_engine::getBitmapSize(data->imageStats), 16);
    game_memory::MemoryBlock result = game_memory::alloc(pArena, hdr_size + data_size, game_memory::AlignNoClear(16));

    game_render_engine::GfxTexture * copy = (game_render_engine::GfxTexture *)result.m_pMemory;
    *copy = *data;
    copy->data = result.m_pMemory + hdr_size;

    uint32 data_len = game_render_engine::getBitmapSize(data->imageStats);
    uint32* dst = (uint32*)copy->data;
    uint32* src = (uint32*)data->data;
    for (uint32 i = 0; i < data_len; ++i) {
        *dst++ = *src++;
    }

    return (result);
}

namespace game_buffer_func {
    uint32 ReadBuffer(rc_cyclic_buffer* buffer, uint32 length, uint8* dst) {
        int32 d = ((int32)buffer->writePosition - (int32)buffer->readPosition);
        if (buffer->readPosition > buffer->writePosition) {
            d += buffer->length;
        }

        uint32 len = _Min(length, (uint32)d);
        if (len) {
            uint8* src = buffer->buffer + buffer->readPosition;
            uint32 t1 = _Min(buffer->length, buffer->readPosition + len);
            uint32 t2 = _Max(0, (int32)(buffer->readPosition + len) - (int32)buffer->length);
            for (uint32 i = t1; i; --i) {
                *dst++ = *src++;
            }
            src = buffer->buffer;
            for (uint32 i = t2; i; --i) {
                *dst++ = *src++;
            }
        }
        return len;
    }

    uint32 WriteBuffer(rc_cyclic_buffer* buffer, uint32 length, uint8* src, uint32 srcLength) {
        uint32 iWriteOffset = 0;
        return iWriteOffset;
    }
}

//------------------------------------------------------------------------------------------------------------------
// Generic pointer LIST functions
//
namespace rc_list {
	block<TEntityReference>* list<TEntityReference>::freeBlock = NULL;
	block<Entity>* list<Entity>::freeBlock = NULL;
}

//
//--------------------------------------------------------------------------------------------------------------------------------------------------
// Generic pointer HASH functions
//
namespace hash_map {
	block<game_map_structs::WorldArea> * map<game_map_structs::WorldArea>::_firstFree = NULL;
	block<EntityPieceReference> * map<EntityPieceReference>::_firstFree = NULL;
}

