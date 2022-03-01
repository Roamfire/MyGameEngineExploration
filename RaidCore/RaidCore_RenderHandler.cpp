/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "../RaidCore/Headers/RaidCore_GameState.h"
#include "../RaidCore/Headers/RaidCore_RenderHandler.h"

namespace render_const {
    __m128i MaskFF = _mm_set1_epi32(0xff);
    __m128i MaskFFFF = _mm_set1_epi32(0xFFFF);
    __m128i MaskFF00FF = _mm_set1_epi32(0x00FF00FF);
    __m128 One = _mm_set1_ps(1.0f);
    __m128 Zero = _mm_set1_ps(0.0f);
    __m128 One255_4x = _mm_set1_ps(255.0f);
    __m128 Inv255_4x = _mm_set1_ps(1.0f / 255.0f);

    const vec2 screenUnitDimention = v2(16.0f, 9.0f); // STATIC value set at game start, equal to 16x9 for a 1920x1080 screen
    const vec2 viewportPixelDimention = v2(1920.0f, 1080.0f); // STATIC value set at game start, equal to 1920x1080 screen

    const real32 monitorWidth = 0.635f;

    GfxColor defaultColor = rgba01(255.0f, 255.0f, 255.0f, 255.0f);
}

namespace game_render_engine {
    RaidCore_EDisplayWindowState m_displayState = e_displayState_Windowed;
    //
    void initTargetBuffers(GameState* gameState, RenderTargetBuffers* targetBuffer) {
        // max window: 1920*2 x 1080 == 3840x1080
        targetBuffer->renderBufferArena = game_memory::createMemoryArena(&gameState->engineArena, MByte(16));
    }
    void initOffscreenBuffer(RenderTargetBuffers* targetBuffer, GfxOffscreenBuffer*& osBuffer, uint32 width, uint32 height, uint32 bpp) {
        osBuffer->imageStats.width = width;
        osBuffer->imageStats.height = height;
        // NOTE(Roman): Since we use 4bytes per pixel, we never need to align to a multiple of 4.
        osBuffer->imageStats.pitch = width * (bpp >> 3);// safeCast2U8((((bpp * width + 31) >> 5) << 2) - width * (bpp >> 3));
        osBuffer->imageStats.bytesPerPixel = (bpp>>3);
        osBuffer->buffer = PushArray(targetBuffer->renderBufferArena, getBitmapSize(osBuffer->imageStats), uint8, game_memory::AlignNoClear(16));
    }
    //
    //
    void* pushRenderElement_(RenderGroup *Group, uint32 Size, render_group_entry_type Type) {
        Assert(Group->InsideRender);

        void* Result = NULL;
        Size += sizeof(RenderGroupEntryHeader);
        if ((Group->PushBufferSize + Size) < Group->MaxPushBufferSize) {
            RenderGroupEntryHeader *Header = (RenderGroupEntryHeader *)(Group->PushBufferBase + Group->PushBufferSize);
            Header->type = Type;
            Header->length = (uint8)(Size - sizeof(RenderGroupEntryHeader));
            Result = (uint8*)Header + sizeof(*Header);
            Group->PushBufferSize += Size;
        } else {
            AssertInvalidCodePath;
        }
        return(Result);
    }
    void* pushRenderChildElement_(RenderGroup *Group, uint32 Size, render_group_entry_type Type) {
        Assert(Group->InsideRender);

        void* Result = NULL;
        if ((Group->PushBufferSize + Size) < Group->MaxPushBufferSize) {
            Result = (uint8 *)(Group->PushBufferBase + Group->PushBufferSize);
            Group->PushBufferSize += Size;
        } else {
            AssertInvalidCodePath;
        }
        return(Result);
    }
    //
    rc_macro_cpp real32 getPerspectiveScale(real32 focalLength, real32 distanceFromTarget) {
        real32 result = game_math::safeRatio1(focalLength, distanceFromTarget);
        return (result);
    }
    //
    RenderEntryTransform getRenderEntityTransform(RenderTransform* transform, const vec3& offset) {
        RenderEntryTransform result;
        result.valid = false32;

        vec3 P = transform->offset;
        P.x += offset.x;
        P.z += offset.z;

        if (transform->Orthographic) {
            real32 perspectiveScale = game_math::safeRatio1(transform->focalLength, transform->distanceAboveTarget);
            result.scale = perspectiveScale * transform->unitsToPixels;
            result.p.xy = transform->screenCenter + v2(P.x, -P.z) * result.scale;
            result.p.z = 0.f;
            result.valid = true32;
        } else {

            real32 DistanceToPZ = (transform->distanceAboveTarget - P.y);
            real32 perspectiveScale = game_math::safeRatio1(transform->focalLength, DistanceToPZ);
            real32 zNear = 0.2f;

            if (DistanceToPZ > zNear) {
                result.scale = perspectiveScale * transform->unitsToPixels;
                result.p.xy = transform->screenCenter + v2(P.x, -P.z) * result.scale;
                result.p.z = offset.y * result.scale;
                result.valid = true32;
            }
        }

        return (result);
    }
}

namespace game_render_scene {
    vec3 canonizeRenderPoint(const real32 groundHeight, const vec3& renderOffset, real32* groundOffset) {
        vec3 renderPoint = renderOffset;

        real32 negate = real32(renderPoint.y < 0.0f) - real32(renderPoint.y >= 0.0f);
        real32 y = game_math::absolute(mathf::floorf(renderPoint.y / groundHeight));
        *groundOffset = renderPoint.y + negate * y * groundHeight;
        renderPoint.y -= *groundOffset;
        return (renderPoint);
    }
	bool32 operator <(const GfxRenderUnit& left, const GfxRenderUnit& right) {
		if (game_math::absolute(left.distanceFromCamera - right.distanceFromCamera) <= game_math::epsilon) {
			return (left.renderPoint.x < right.renderPoint.x);
		}
		return (left.distanceFromCamera < right.distanceFromCamera);
	}
	bool32 operator <=(const GfxRenderUnit& left, const GfxRenderUnit& right) {
		if (game_math::absolute(left.distanceFromCamera - right.distanceFromCamera) <= game_math::epsilon) {
			return (left.renderPoint.x <= right.renderPoint.x);
		}
		return (left.distanceFromCamera <= right.distanceFromCamera);
	}
    rc_macro_h bool32 operator <=(GfxRenderBlock& left, GfxRenderBlock& right) {
		return (*right.renderEntry <= *left.renderEntry);
    }
    rc_macro_h bool32 operator >(GfxRenderBlock& left, GfxRenderBlock& right) {
		return !(*right.renderEntry < *left.renderEntry);
    }

    namespace render_sort {
        //
        //
        //  Left half is A[iBegin:iMiddle-1].
        // Right half is A[iMiddle:iEnd-1   ].
        void RenderVolumeArrayMerge(GfxRenderBlock** A, uint32 iBegin, uint32 iMiddle, uint32 iEnd, GfxRenderBlock** B) {
            uint32 i = iBegin, j = iMiddle;
            // While there are elements in the left or right runs...
            for (uint32 k = iBegin; k < iEnd; ++k) {
                GfxRenderBlock*& Bk = *(B + k);

                // If left run head exists and is <= existing right run head.
                //if (i < iMiddle && (j >= iEnd || Ai <= Aj)) {
                if (i < iMiddle && j < iEnd) {
                    GfxRenderBlock*& Ai = *(A + i);
                    GfxRenderBlock*& Aj = *(A + j);
                    if (*Ai <= *Aj) {
                        Bk = Ai;
                        ++i;
                    } else {
                        Bk = Aj;
                        ++j;
                    }
                } else if (i < iMiddle) {
                    Bk = *(A + i);
                    ++i;
                } else if (j < iEnd) {
                    Bk = *(A + j);
                    ++j;
                }
            }
            for (uint32 k = iBegin; k < iEnd; ++k) {
                *(A + k) = *(B + k);
            }
        }
        //
        // iBegin is inclusive; iEnd is exclusive (A[iEnd] is not in the set).
        void RenderVolumeSplitAndMerge(GfxRenderBlock** A, uint32 iBegin, uint32 iEnd, GfxRenderBlock** B) {
            if (iEnd - iBegin <= 1) {                   // if run size == 1
                return;                                 //   consider it sorted
            } else if (iEnd - iBegin == 2) {
                GfxRenderBlock*& Ai = *(A + iBegin);
                GfxRenderBlock*& Aj = *(A + iBegin + 1);
                if (*Ai > *Aj) {
                    SwapMemBlock<GfxRenderBlock*>(A + iBegin, A + iBegin + 1);
                }
                return;
            }
            // recursively split runs into two halves until run size == 1,
            // then merge them and return back up the call chain
            uint32 iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
            RenderVolumeSplitAndMerge(A, iBegin, iMiddle, B);  // split / merge left  half
            RenderVolumeSplitAndMerge(A, iMiddle, iEnd, B);  // split / merge right half
            RenderVolumeArrayMerge(A, iBegin, iMiddle, iEnd, B);  // merge the two half runs
        }
        //
        // Array A[] has the items to sort; array B[] is a work array.
        void mergePointerSort(GfxRenderBlock** inArray, game_memory::arena_p tempMemory, uint32 arrayLength) {
            TemporaryMemory _t = BeginTemporaryMemory(tempMemory);
            GfxRenderBlock** temporary = PushArray(_t.arena, arrayLength, GfxRenderBlock*);
            RenderVolumeSplitAndMerge(inArray, 0, arrayLength, temporary);
            EndTemporaryMemory(_t);
        }
    }

    // NOTE(Roman): render space is the screen partitioned to 16x16 squares
    //  each drawing element we want fits into one of those, according to its base point (x+width/2,y)
    static const int32 width_parts = 16;
    static const int32 height_parts = 9;
    RenderSpace* InitRenderSpace(GameState* gameState, real32 maxWidth, real32 maxLength) {
        RenderSpace* result = (RenderSpace*)PushSizeP(&gameState->scratchFrameArena, sizeof(GfxRenderBlock*) * height_parts * width_parts + sizeof(RenderSpace)); // PushStruct(&gameState->scratchFrameArena, RenderSpace);
        result->viewVolumes = (GfxRenderBlock**)(result + 1);
        //for (uint32 i = 0; i < ArrayCount(result->viewVolumes); ++i) {
        //    result->viewVolumes[i].SortedRenderList = rc_list::initList<GfxRenderUnit>();
        //}
        result->halfWidth = maxWidth * 0.5f;
        result->halfHeight = maxLength * 0.5f;
        //
        result->nextReadOffset = 0;
        return (result);
    }

    GfxRenderUnit* InsertRenderCommand(GameState* gameState, RenderSpace* renderSpace, EntityPiece* src, vec3& center, real32 yOffset, GfxRenderUnit* next) {
        GfxRenderUnit* result = NULL;
        if (src && ((AssetType)src->iEntityAsset != AssetType_None)) {
            //
            vec3 virtual_cam = v3(center.x, 4.0f * gameState->cameraEntity.distanceAboveTarget, 16.f);
            real32 fDistance = game_math::lengthSq(center - virtual_cam);

            // NOTE(Roman): the real scale factor is the ratio between the perspective scale at distance from target and distance from camera level
            // this is to nullify the distance from camera level
            real32 scaleFactor = game_math::safeRatio1(
                game_render_engine::getPerspectiveScale(gameState->cameraEntity.focalLength, gameState->cameraEntity.distanceAboveTarget - center.y),
                game_render_engine::getPerspectiveScale(gameState->cameraEntity.focalLength, gameState->cameraEntity.distanceAboveTarget)
                );

            EntityPiece* e = src;
            int32 x = -1;
            int32 z = -1;
            while (e) {
                // NOTE(Roman): convert x,z to 1-16 x 1-9
                real32 xCoefficient = (real32)width_parts / ((renderSpace->halfWidth + src->iEntityBounds.radialDimention.x) * 2.0f);
                real32 zCoefficient = (real32)height_parts / ((renderSpace->halfHeight + src->iEntityBounds.radialDimention.z) * 2.0f);

                real32 cx = center.x * scaleFactor;
                real32 cz = center.z * scaleFactor;
                cz -= yOffset; // NOTE(Roman): yOffset directly affects z, so update the center to reflect this offset

                int32 x = game_math::floorToInt32((cx + renderSpace->halfWidth) * xCoefficient);
                int32 z = game_math::floorToInt32((cz + renderSpace->halfHeight) * zCoefficient);

                if (x >= width_parts || x < 0 || z >= height_parts || z < 0) {
                    e = e->next;
                    continue;
                }
                break;
            }

            if (e) {
                x = game_math::clamp(0, x, width_parts - 1);
                z = game_math::clamp(0, z, height_parts - 1);

                GfxRenderBlock*& renderVolume = renderSpace->viewVolumes[x + z * width_parts];

                result = PushStruct(&gameState->scratchFrameArena, GfxRenderUnit);
                result->distanceFromCamera = fDistance;
                result->renderPoint = center;
                result->renderSource = src;
                result->offsetOnY = yOffset;
                //
                GfxRenderBlock* newAdd = PushStruct(&gameState->scratchFrameArena, GfxRenderBlock);
                GfxRenderBlock* render1 = renderVolume;

                newAdd->renderEntry = result;

                newAdd->next = render1;
                renderVolume = newAdd;
                //
                while (render1 && (*newAdd->renderEntry < *render1->renderEntry)) {
                    GfxRenderUnit* t = newAdd->renderEntry;
                    newAdd->renderEntry = render1->renderEntry;
                    render1->renderEntry = t;

                    newAdd = render1;
                    render1 = render1->next;
                }
                ++renderSpace->count;
            }
        }
        return (result);
    }
	void OrderRenderVolumes(game_memory::arena* arena, RenderSpace* renderSpace) {
		// NOTE(Roman): compact the array
        if (renderSpace->count) {
            uint32 last = 0;
            renderSpace->volumeCount = width_parts * height_parts;
            for (uint32 i = 0; i < width_parts*height_parts; ++i) {
                if (renderSpace->viewVolumes[i] == NULL) {
                    --renderSpace->volumeCount;
                } else {
                    while (last < i && renderSpace->viewVolumes[last]) ++last;
                    if (last < i) {
                        SwapMemBlock<GfxRenderBlock*>(&renderSpace->viewVolumes[i], &renderSpace->viewVolumes[last++]);
                    }
                }
            }
            render_sort::mergePointerSort(renderSpace->viewVolumes, arena, renderSpace->volumeCount);
        }
        renderSpace->nextReadOffset = 0;
	}
    GfxRenderUnit* GetNextRenderCommand(RenderSpace* renderSpace) {
        GfxRenderUnit* result = NULL;

        if (renderSpace->nextReadOffset < (width_parts * height_parts) && renderSpace->viewVolumes[renderSpace->nextReadOffset]) {
			result = renderSpace->viewVolumes[renderSpace->nextReadOffset]->renderEntry;
			renderSpace->viewVolumes[renderSpace->nextReadOffset] = renderSpace->viewVolumes[renderSpace->nextReadOffset]->next;
			--renderSpace->count;
			if (NULL == renderSpace->viewVolumes[renderSpace->nextReadOffset]) {
				++renderSpace->nextReadOffset;
			}

			uint32 i = renderSpace->nextReadOffset;
			uint32 nextI = i + 1;
            while (renderSpace->viewVolumes[nextI] && nextI < (width_parts * height_parts)) {
				if (*renderSpace->viewVolumes[i]->renderEntry < *renderSpace->viewVolumes[nextI]->renderEntry) {
					SwapMemBlock<GfxRenderBlock*>(&renderSpace->viewVolumes[i++], &renderSpace->viewVolumes[nextI++]);
                    continue;
				}
				break;
			}
		}
        return (result);
    }
}
