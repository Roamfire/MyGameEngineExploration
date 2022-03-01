/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_OPENGL_RENDERER_H_
#ifdef __USE_OPENGL__
#ifndef GLEW_STATIC
#define GLEW_STATIC 1
#endif

#include "GL/glew.h"
#include "GL/wglew.h"

#include "../OSHeaders/RaidCore_Win32_Struct.h"

namespace game_render_engine {
    enum gpuItemType {
        Type_QuadWireframe = 0, // 3d wireframe (no texture)
        Type_QuadWireframeBB, // billboard wireframe (no texture)
        Type_QuadRect,      // 3d rectangle (color, solid or gradient)
        Type_QuadRectBB,      // billboard rectangle (color, solid or gradient)
        Type_QuadTexture,   // 3d textured quad
        Type_QuadBillboard, // billboard textured quad

        Type_QuadCount
    };

    namespace quad {

        struct GfxQuad {
            GLuint textureId;
            GLuint sampleId;
            GLuint vertexArrayObject;
            GLuint vertexBufferObjectId; // coords
#ifdef SEPARATE_VB
            GLuint textureBufferObjectId; // texture coords
#endif
            GLuint programId;
        };
        void pushTextureQuad(GameRenderTarget* targetPtr, BitmapID* id, const vec3& offset, const vec3& scale);
    }

    enum gpuEntityType {
        gpuEntityType_Quad,
    };
    struct GfxGpuEntity {
        BitmapID id;
        gpuEntityType type;
        //
        union {
            struct {
                quad::GfxQuad quadTexture;
                GLuint quadElementsId;
            };
        };
    };

    bool32 LoadImageToGPU(GfxTexture& tex);
    /******************************************************************
    * Game subsystems resources
    */
    struct glShader {
        GLuint id;
        uint32 type;
        bool32 loaded;
    };
    struct glProgram {
        GLuint id;
        uint32 shaderCount;
        glShader shaders[8];
        bool32 loaded;
    };

    struct GameRenderTarget {
        HGLRC
            glDeviceContext;
        HDC
            hDC;

        mat4x4
            glProjection;
        mat4x4
            glOrtographic;

        PlatformWindow
            window;
        bool32
            initialized;

        RenderTargetBuffers
            targetBuffers;
        GfxOffscreenBuffer
            *offscreen_buffer;
        //
        game_memory::arena_p textureScratchMemory;
        //
        hash_map::map<GfxGpuEntity> itemMap;
        glProgram gpuPrograms[Type_QuadCount];
        //
        RenderGroup* renderGroup;
    };
}
#endif//__USE_OPENGL__
#define __RC_OPENGL_RENDERER_H_
#endif//__RC_OPENGL_RENDERER_H_