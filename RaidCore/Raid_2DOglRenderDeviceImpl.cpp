/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __GC_2DOgl_RENDER_DEVICE_H_

// --
#include "../RaidCore/Headers/RaidCore_GameState.h"
#include "../RaidCore/Headers/RaidCore_RenderHandler.h"

#include "../GameHandler/debug_s.h"

#ifdef __USE_OPENGL__
// OpenGl
#include "../RaidCore/RaidCore_Ogl2DUtils.cpp"
#include "../RaidCore/Headers/RaidCore_MathOperators.h"

// SHADERS
char* Shader01 = R"S1(
  //Shader code here!!!
)S1";

// Simple vertex shader
const GLchar* VertexShader01 = R"glsl(
    #version 330
    uniform mat4 modelViewMatrix;
    layout (location = 0) in vec3 inPosition;
    layout (location = 1) in vec2 inCoord;
    out vec3 outColor;
    out vec2 texCoord;
    void main() {
        gl_Position = modelViewMatrix * vec4(inPosition, 1.0);
        texCoord = inCoord;
    }
)glsl";

// Simple fragment shader
const GLchar* FragmentShader01 = R"glsl(
    #version 330
    in vec3 inColor;
    in vec2 texCoord;
    out vec4 outputColor;
    uniform sampler2D gSampler;
    void main() {
        outputColor = texture(gSampler, texCoord);
    }
)glsl";

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "Gdi32.lib")

namespace {
    class Program {
    private:
        uint32 count;
    };
}

namespace hash_map {
    block<game_render_engine::GfxGpuEntity> * map<game_render_engine::GfxGpuEntity>::_firstFree = NULL;
}

namespace game_render_engine
{
    int32 majorVersion = 4;
    int32 minorVersion = 4;

    void recalculateProjectionMatrix(GameState* pState) {
        pState->renderTargetPtr->glProjection = perspectiveProjection(pState->screenAspectRatio, pState->cameraEntity.focalLength, 0.5f, 1000.0f);
        pState->renderTargetPtr->glOrtographic = orthographicProjection((f32)pState->screenPixelResolution.x, (f32)pState->screenPixelResolution.y);
    }

    static void RenderScene(void* lpParam) {
        GameRenderTarget* targetPtr = (GameRenderTarget*)lpParam;
        // 
        // [Scene render]
        // Pull camera
        // Set camera transforms
        // Loop over render groups (should be sorted back to front)
        //  Initiate transform matrix
        //   In each render group, Loop over commands
        //     Dequeue render command
        //     modify transform matrix (cumulative)
        //     set drawing geometry and draw
        //   next command
        // next render group

        uint8* command = targetPtr->renderGroup->PushBufferBase;
        uint32 processed = 0;

        RenderGroupEntryHeader* header;
        while (processed < targetPtr->renderGroup->PushBufferSize) {
            processed += sizeof(RenderGroupEntryHeader);
            header = (RenderGroupEntryHeader*)command;
            command += sizeof(RenderGroupEntryHeader);
            switch (header->type) {
                case RenderGroupEntryType_RenderEntryClear: {
                    processed += sizeof(RenderEntryClear);
                    // DO something!!
                    glClear(GL_COLOR_BUFFER_BIT);
                    // continue;
                    command += sizeof(RenderEntryClear);
                    break;
                }
                case RenderGroupEntryType_RenderEntryBitmap:{
                    processed += sizeof(RenderEntryBitmap);
                    RenderEntryBitmap* drawBitmap = (RenderEntryBitmap*)command;
                    // DO something!!

                    // continue;
                    command += sizeof(RenderEntryBitmap);
                    break;
                }
            }
        }

        uint32 count = 0;// targetPtr->itemMap._blockCount;
        while (count) {
            GfxGpuEntity* gpuItem=0L;

            glUseProgram(gpuItem->quadTexture.programId);

            glBindVertexArray(gpuItem->quadTexture.vertexArrayObject);

            mat4x4 matOff = game_math::identity();
            matOff = game_math::transpose(matOff);
            mat4x4 matx = game_math::identity();
            //rotate(matx, scale.y, v4(0.0f, 1.0f, 0.0f, 0.0f));
            matx = game_math::transpose(matx);

            //bindTexture(tex, 0);
            //int iSamplerLoc = glGetUniformLocation(gpuItem->programId, "gSampler");
            //glUniform1i(iSamplerLoc, 0);

            //int iModelViewLoc = glGetUniformLocation(gpuItem->programId, "modelViewMatrix");
            //glUniformMatrix4fv(iModelViewLoc, 1, GL_FALSE, (matOff*matx).E[0]);

#if 1
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
#else
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
        }

        SwapBuffers(targetPtr->hDC);
    }

    static bool32 InitOpenGL(GameState* pState, int iMajorVersion, int iMinorVersion, LPVOID lpParam) {

        HINSTANCE hInstance = NULL;
        GameRenderTarget* targetPtr = pState->renderTargetPtr;
        if (!targetPtr->initialized) {
            //RegisterSimpleOpenGLClass(hInstance);

            HWND hWndFake = CreateWindow(RC_OPENGL_CLASS_NAME, "FAKE", WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_CLIPCHILDREN,
                                         0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
                                         NULL, hInstance, NULL);

            HDC hDC = GetDC(hWndFake);

            // First, choose false pixel format

            PIXELFORMATDESCRIPTOR pfd;
            memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
            pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 32;
            pfd.iLayerType = PFD_MAIN_PLANE;

            int iPixelFormat = ChoosePixelFormat(hDC, &pfd);
            if (iPixelFormat == 0) return false32;

            if (!SetPixelFormat(hDC, iPixelFormat, &pfd)) return false32;

            // Create the false, old style context (OpenGL 2.1 and before)
            HGLRC hRCFake = wglCreateContext(hDC);
            wglMakeCurrent(hDC, hRCFake);

            bool32 bResult = true32;

            if (!targetPtr->initialized) {
                if (glewInit() != GLEW_OK) {
                    bResult = false32;
                }
                targetPtr->initialized = true32;
            }

            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(hRCFake);
            ReleaseDC(hWndFake, hDC);
            DestroyWindow(hWndFake);

            if (!bResult) {
                return false32;
            }
        }

        if (!createWindow(0, 0, RC_OPENGL_CLASS_NAME, targetPtr->window, WS_OVERLAPPEDWINDOW | WS_VISIBLE)) {
            return false32;
        }

        targetPtr->hDC = GetDC(targetPtr->window.hWnd);

        bool bError = false;
        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd  
            1,                     // version number  
            PFD_DRAW_TO_WINDOW |   // support window  
            PFD_SUPPORT_OPENGL |   // support OpenGL  
            PFD_DOUBLEBUFFER,      // double buffered  
            PFD_TYPE_RGBA,         // RGBA type  
            24,                    // 24-bit color depth  
            0, 0, 0, 0, 0, 0,      // color bits ignored  
            0,                     // no alpha buffer  
            0,                     // shift bit ignored  
            0,                     // no accumulation buffer  
            0, 0, 0, 0,            // accum bits ignored  
            32,                    // 32-bit z-buffer  
            0,                     // no stencil buffer  
            0,                     // no auxiliary buffer  
            PFD_MAIN_PLANE,        // main layer  
            0,                     // reserved  
            0, 0, 0                // layer masks ignored  
        };

        if (iMajorVersion <= 2) {
            memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
            pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 32;
            pfd.iLayerType = PFD_MAIN_PLANE;

            int iPixelFormat = ChoosePixelFormat(targetPtr->hDC, &pfd);
            if (iPixelFormat == 0) {
                return false;
            }

            if (!SetPixelFormat(targetPtr->hDC, iPixelFormat, &pfd)) {
                return false;
            }

            // Create the old style context (OpenGL 2.1 and before)
            targetPtr->glDeviceContext = wglCreateContext(targetPtr->hDC);
            if (targetPtr->glDeviceContext) {
                wglMakeCurrent(targetPtr->hDC, targetPtr->glDeviceContext);
            } else {
                bError = true;
            }
        } else if (WGLEW_ARB_create_context && WGLEW_ARB_pixel_format) {
            const int iPixelFormatAttribList[] =
            {
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                WGL_COLOR_BITS_ARB, 32,
                WGL_DEPTH_BITS_ARB, 24,
                WGL_STENCIL_BITS_ARB, 8,
                0 // End of attributes list
            };
            int iContextAttribs[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, iMajorVersion,
                WGL_CONTEXT_MINOR_VERSION_ARB, iMinorVersion,
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                0 // End of attributes list
            };

            int iPixelFormat, iNumFormats;
            wglChoosePixelFormatARB(targetPtr->hDC, iPixelFormatAttribList, NULL, 1, &iPixelFormat, (UINT*)&iNumFormats);

            // PFD seems to be only redundant parameter now
            if (!SetPixelFormat(targetPtr->hDC, iPixelFormat, &pfd)) {
                return false;
            }

            targetPtr->glDeviceContext = wglCreateContextAttribsARB(targetPtr->hDC, 0, iContextAttribs);
            // If everything went OK
            if (targetPtr->glDeviceContext) {
                wglMakeCurrent(targetPtr->hDC, targetPtr->glDeviceContext);
            } else {
                bError = true;
            }
        } else {
            bError = true;
        }

        if (bError) {
            // Generate error messages
            return false32;
        }
        return true32;
    }


    /******************************************************************
    * Run functions
    */

    /******************************************************************
    * Game subsystems resources
    */
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

        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        targetPtr->initialized = false32;
        targetPtr->offscreen_buffer = &window->offscreen_buffer;
        initTargetBuffers(gameState, &targetPtr->targetBuffers);

        targetPtr->window = *window;

        if (!InitOpenGL(gameState, majorVersion, minorVersion, targetPtr)) {
            return false32;
        }

        hash_map::initialize<GfxGpuEntity>(&targetPtr->itemMap, &gameState->engineArena, 512);

        ShowWindow(targetPtr->window.hWnd, SW_SHOW);

        // Just to send WM_SIZE message again
        ShowWindow(targetPtr->window.hWnd, SW_SHOWMAXIMIZED);
        UpdateWindow(targetPtr->window.hWnd);

        RECT rRect; GetClientRect(targetPtr->window.hWnd, &rRect);
        glViewport(0, 0, rRect.right, rRect.bottom);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        *window = targetPtr->window;
        return true32;
    }
    void freeRenderTarget(GameState* gameState) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        Assert(targetPtr);

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(targetPtr->glDeviceContext);
        ReleaseDC(targetPtr->window.hWnd, targetPtr->hDC);
    }

    void resizeRenderer(GameState* gameState, uint32 width, uint32 height) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
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
    // two functions,
    // - startRenderProcess() will setup the render state;
    //	. clear back buffer
    //	. setup rendering states like position, draw methods etc...
    RenderGroup* startRenderProcess(GameState* gameState) {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;

        // NOTE(Roman): is here a good place to load shaders?
//        uint32 vertexId = loadSpriteVertexShader();
//        uint32 fragmentId = loadSpriteFragmentShader();

        targetPtr->renderGroup = createRenderGroup(gameState, &gameState->scratchFrameArena);
        return (gameState->renderTargetPtr->renderGroup);
    }
    //
    // - completeRender() will finish up, launch the rendering process and flip buffers on the flip chain.
    bool32 completeRender(GameState* gameState)
    {
        GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        // render!!!
        RenderScene(targetPtr);
        return false32;
    }

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
    // Here are the different drawing methods...

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
            data->color = rgba01(rgba_unpack4x8(color));
        }
        glClearColor(data->color.r, data->color.g, data->color.b, data->color.a);
    }

    void pushRect(RenderGroup *renderGroup, const vec3& offset, const vec3& rectDim, bool32 fill, const GfxColor color) {
        /*GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        RenderGroup* renderGroup = targetPtr->renderGroup;
        if (!hash_map::isSet<gfx_Brush>(&targetPtr->brushMap, colorHashKey(color))) {
            gfx_Brush b = { color, ::CreateSolidBrush(color) };
            hash_map::add<gfx_Brush>(&targetPtr->brushMap, b, colorHashKey(color));
        }
        RenderEntryRect* data = (RenderEntryRect*)pushRenderElement(renderGroup, RenderEntryRect);
        if (NULL != data) {
            data->color = rgba_unpack4x8(color);
            data->size = getSize(rect);
            data->p = offset;
        }*/
    }
    void pushImage(GameState* gameState, BitmapID id, const vec3& offset, real32 resize, const vec4& color) {
        /*GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        RenderGroup* renderGroup = targetPtr->renderGroup;
        RenderEntryBitmap* bitmap = (RenderEntryBitmap*)pushRenderElement(renderGroup, RenderEntryBitmap);
        if (NULL != bitmap) {
            bitmap->color = color;
            bitmap->p = offset;
            GfxBox rc = { bottomLeft, topRight };
            bitmap->size = getSize(rc).xy;
            if (!hash_map::isSet(&targetPtr->textureMap, id.Value)) {
                gfx_textureResource res = {};
                res.resourceId = id.Value;
                res.image = gameState->_loadedBitmaps[id.Value];
                hash_map::add(&targetPtr->textureMap, res, id.Value);
            }
            hash_map::get(&targetPtr->textureMap, id.Value, bitmap->bitmap);
        }*/
    }
    bool32 pushSprite(RenderGroup *renderGroup, GfxTexture* tex, real32 height, const vec3& offset, const GfxColor color) {
        /*GameRenderTarget* targetPtr = gameState->renderTargetPtr;
        RenderGroup* renderGroup = targetPtr->renderGroup;
        RenderEntryBitmap* bitmap = (RenderEntryBitmap*)pushRenderElement(renderGroup, RenderEntryBitmap);
        if (NULL != bitmap) {
        bitmap->color = color;
        bitmap->p = offset;
        GfxBox rc = { bottomLeft, topRight };
        bitmap->size = getSize(rc).xy;
        if (!hash_map::isSet(&targetPtr->textureMap, id.Value)) {
        gfx_textureResource res = {};
        res.resourceId = id.Value;
        res.image = gameState->_loadedBitmaps[id.Value];
        hash_map::add(&targetPtr->textureMap, res, id.Value);
        }
        hash_map::get(&targetPtr->textureMap, id.Value, bitmap->bitmap);
        }*/
        return true32;
    }

#if 0
    void RenderQuad(GameRenderTarget* targetPtr, const vec3& tL, const vec3& bR, const vec3& offset, const vec3& scale) {
        GfxGpuEntity* rect = NULL;
        GfxTexture tex = {};
        bool32 found = hash_map::get(&targetPtr->itemMap, 0, rect);

        if (!found) {
            rect = hash_map::add(&targetPtr->itemMap, GfxGpuEntity(), 0);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glGenVertexArrays(1, &rect->vaoId);
            glGenBuffers(1, rect->vboArray);
            rect->vboCount = 1;

            glBindVertexArray(rect->vaoId);
            
            float xZ_m = game_math::lerp(bR.z, 0.5f, tL.z);
#if 0
            float fQuad[] = {
                // position       // colors         // texture u,v
                bR.x, tL.y, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // Top Right
                bR.x, bR.y, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // Bottom Right
                tL.x, bR.y, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // Bottom Left
                tL.x, tL.y, -0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f   // Top Left
            };
#else
            float fQuad[] = {
                // position        // texture u,v
                bR.x, tL.y, -0.5f, 1.0f, 1.0f,  // Top Right
                bR.x, bR.y, xZ_m,  1.0f, 0.0f,  // Bottom Right
                tL.x, bR.y, xZ_m,  0.0f, 0.0f,  // Bottom Left
                tL.x, tL.y, tL.z,  0.0f, 1.0f   // Top Left
            };
#endif

            glBindBuffer(GL_ARRAY_BUFFER, rect->vboArray[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(fQuad), fQuad, GL_STATIC_DRAW);

            rect->shaderArray[0] = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(rect->shaderArray[0], 1, &VertexShader01, NULL);
            glCompileShader(rect->shaderArray[0]);
            rect->shaderArray[1] = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(rect->shaderArray[1], 1, &FragmentShader01, NULL);
            glCompileShader(rect->shaderArray[1]);

            int iCompilationStatus;
            glGetShaderiv(rect->shaderArray[0], GL_COMPILE_STATUS, &iCompilationStatus);
            glGetShaderiv(rect->shaderArray[1], GL_COMPILE_STATUS, &iCompilationStatus);
            
            rect->programId = glCreateProgram();
            glAttachShader(rect->programId, rect->shaderArray[0]);
            glAttachShader(rect->programId, rect->shaderArray[1]);

            glLinkProgram(rect->programId);
            glGetProgramiv(rect->programId, GL_LINK_STATUS, &iCompilationStatus);

            glUseProgram(rect->programId);
            rect->shaderCount = 2;

#if 0
            GLuint indices[] = {
                0, 1, 3,
                1, 2, 3
            };
            glGenBuffers(1, &rect->indicesId);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect->indicesId);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            GLint posAttrib = glGetAttribLocation(rect->programId, "inPosition");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(real32), 0);

            GLint colAttrib = glGetAttribLocation(rect->programId, "inColor");
            glEnableVertexAttribArray(colAttrib);
            glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(real32), (void*)(sizeof(real32) * 3));

            GLint coordAttrib = glGetAttribLocation(rect->programId, "inCoord");
            glEnableVertexAttribArray(coordAttrib);
            glVertexAttribPointer(coordAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(real32), (void*)(sizeof(real32) * 6));
#else
            GLuint indices[] = {
                0, 1, 3,
                1, 2, 3
            };
            glGenBuffers(1, &rect->indicesId);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect->indicesId);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            GLint posAttrib = glGetAttribLocation(rect->programId, "inPosition");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(real32), 0);

            GLint coordAttrib = glGetAttribLocation(rect->programId, "inCoord");
            glEnableVertexAttribArray(coordAttrib);
            glVertexAttribPointer(coordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(real32), (void*)(sizeof(real32) * 3));
#endif

            glBindVertexArray(0);

            //glEnable(GL_DEPTH_TEST);
            tex = LoadResourceImage("tree01.bmp", targetPtr->textureMap._hashMemoryAlloc, false32);

            rect->textureId = tex.textureId;
            gfx_sys_texture st = {};
            st.resource.image = tex;
            st._glId = tex.textureId;

            hash_map::add<gfx_sys_texture>(&targetPtr->textureMap, st, st._glId - 1);
            hash_map::add<GfxGpuEntity>(&targetPtr->itemMap, *rect, rect->vaoId - 1);

        }

        glUseProgram(rect->programId);

        glBindVertexArray(rect->vaoId);
        gfx_sys_texture* st;
        if (hash_map::get(&targetPtr->textureMap, rect->textureId - 1, st)) {
            tex = st->resource.image;
        }

        mat4x4 matOff = game_math::identity();
        matOff = game_math::transpose(matOff);
        mat4x4 matx = game_math::identity();
        rotate(matx, scale.y, v4(0.0f, 1.0f, 0.0f, 0.0f));
        matx = game_math::transpose(matx);

        bindTexture(tex, 0);
        int iSamplerLoc = glGetUniformLocation(rect->programId, "gSampler");
        glUniform1i(iSamplerLoc, 0);

        int iModelViewLoc = glGetUniformLocation(rect->programId, "modelViewMatrix");
        glUniformMatrix4fv(iModelViewLoc, 1, GL_FALSE, (matOff*matx).E[0]);

#if 1
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
#else
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
    }
    
#endif
}
#endif

#define __GC_2DOgl_RENDER_DEVICE_H_
#endif//__GC_2DOgl_RENDER_DEVICE_H_
