/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef _RC_OPENGL_UTIL_H_
#define _RC_OPENGL_UTIL_H_

#include "Headers\RaidCore_Common.h"
#include "Headers\RaidCore_RenderHandler.h"
#include "Headers\RaidCore_MathOperators.h"
#include "Headers\RaidCore_Platform.h"

#ifdef __USE_OPENGL__
// OpenGl
#include "../RaidCore/Headers/RaidCore_OpenGlRenderer.h"

//
namespace game_render_engine {
    glShader LoadShader(const char* source, uint32 type) {
        glShader result = {};

        //GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
        result.id = glCreateShader(type);
        glShaderSource(result.id, 1, &source, NULL);
        glCompileShader(result.id);

        int iCompilationStatus;
        glGetShaderiv(result.id, GL_COMPILE_STATUS, &iCompilationStatus);

        result.loaded = (iCompilationStatus != GL_FALSE);
        return result;
    }
    void FreeShader(glShader* shader) {
        if (shader && shader->loaded) {
            glDeleteShader(shader->id);
            shader->loaded = false32;
            shader->id = 0;
        }
    }
    glProgram NewProgram() {
        glProgram result = {};
        result.id = glCreateProgram();
        return result;
    }
    bool32 AddShaderToProgram(glProgram* program, glShader* shader) {
        bool32 result = false32;

        if (program && program->shaderCount < ArrayCount(program->shaders)) {
            program->shaders[program->shaderCount++] = *shader;
            result = true32;
        }

        return result;
    }
    bool32 LinkProgram(glProgram* program) {
        int iCompilationStatus;

        if (program && program->shaderCount && !program->loaded) {
            for (uint32 i = 0; i < program->shaderCount; ++i) {
                if (!program->shaders[i].loaded) {

                    glCompileShader(program->shaders[i].id);

                    glGetShaderiv(program->shaders[i].id, GL_COMPILE_STATUS, &iCompilationStatus);

                    program->shaders[i].loaded = (iCompilationStatus != GL_FALSE);
                }
                if (program->shaders[i].loaded) {
                    glAttachShader(program->id, program->shaders[i].id);
                }
            }

            glLinkProgram(program->id);
            glGetProgramiv(program->id, GL_LINK_STATUS, &iCompilationStatus);

            program->loaded = (iCompilationStatus != GL_FALSE);
            if (program->loaded) {
                for (uint32 i = 0; i < program->shaderCount; ++i) {
                    if (program->shaders[i].loaded) {
                        glDeleteShader(program->shaders[i].id);
                        program->shaders[i].loaded = false32;
                        program->shaders[i].id = 0;
                    }
                }
                program->shaderCount = 0;
            }
        }

        return program->loaded;
    }
    void RunProgram(glProgram* program) {
        if (program && program->loaded) {
            glUseProgram(program->id);
        }
    }
    void FreeProgram(glProgram* program) {
        if (program && program->loaded) {
            for (uint32 i = 0; i < program->shaderCount; ++i) {
                if (program->shaders[i].loaded) {
                    glDeleteShader(program->shaders[i].id);
                    program->shaders[i].loaded = false32;
                }
            }
            glDeleteProgram(program->id);
        }
    }

    namespace quad {
        // Simple vertex shader
        const GLchar* VertexShader01 = R"glsl(
            #version 330
            uniform mat4 quadSetMatrix;
            uniform mat4 modelViewMatrix;
            layout (location = 0) in vec3 inPosition;
            layout (location = 1) in vec2 inCoord;
            out vec2 texCoord;
            void main() {
                gl_Position = modelViewMatrix * quadSetMatrix * vec4(inPosition, 1.0);
                texCoord = inCoord;
            }
        )glsl";

        // Simple fragment shader
        const GLchar* FragmentShader01 = R"glsl(
            #version 330
            in vec2 texCoord;
            layout (location = 0) out vec4 outputColor;
            uniform sampler2D gSampler;
            void main() {
                outputColor = texture(gSampler, texCoord);
            }
        )glsl";

        struct TexturedQuadVertex {
            vec3 vCoord;
            vec2 tCoord;
        };
        //------------------------------
        void pushRectQuad(GameRenderTarget* targetPtr, BitmapID* idPtr, const vec3& offset, const vec3& scale, bool32 billboard = false32) {
            gpuItemType type = Type_QuadTexture;
            if (billboard) {
                type = Type_QuadBillboard;
            }

            GfxGpuEntity* gpuItem = NULL;
            GfxTexture tex = {};
            // RenderGroupEntryType_render_entry_bitmap

            if (!hash_map::get(&targetPtr->itemMap, idPtr->Value, gpuItem)) {

                gpuItem = hash_map::add(&targetPtr->itemMap, GfxGpuEntity(), idPtr->Value);
                gpuItem->id = *idPtr;

                gpuItem->type = gpuEntityType_Quad;

                glGenVertexArrays(1, &gpuItem->quadTexture.vertexArrayObject); // NOTE(Roman) : create the VAO
                glBindVertexArray(gpuItem->quadTexture.vertexArrayObject);

#ifdef SEPARATE_VB
                GLuint ids[2];
                glGenBuffers(2, ids);
                gpuItem->quadTexture.vertexBufferObjectId = ids[0];
                gpuItem->quadTexture.textureBufferObjectId = ids[1];

                real32 fQuad[] = {
                    // vertices
                    0.5f, 0.5f, 0.0f,  // Top Right
                    0.5f, -0.5f, 0.0f,  // Bottom Right
                    -0.5f, -0.5f, 0.0f,// Bottom Left
                    -0.5f, 0.5f, 0.0f  // Top Left
                };
                // NOTE(Roman): populate position buffer
                glBindBuffer(GL_ARRAY_BUFFER, gpuItem->quadTexture.vertexBufferObjectId);
                glBufferData(GL_ARRAY_BUFFER, sizeof(fQuad), fQuad, GL_STATIC_DRAW);

                real32 fTex[] = {
                    // texture u,v
                    1.0f, 1.0f,  // Top Right
                    1.0f, 0.0f,  // Bottom Right
                    0.0f, 0.0f,  // Bottom Left
                    0.0f, 1.0f   // Top Left
                };
                // NOTE(Roman): populate texture coords buffer
                glBindBuffer(GL_ARRAY_BUFFER, gpuItem->quadTexture.textureBufferObjectId);
                glBufferData(GL_ARRAY_BUFFER, sizeof(fTex), fTex, GL_STATIC_DRAW);
#else
                glGenBuffers(1, &gpuItem->quadTexture.vertexBufferObjectId);

                TexturedQuadVertex fTexQuad[] = {
                    // vertices
                    0.5f, 0.5f, 0.0f, 1.0f, 1.0f,  // Top Right
                    0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom Right
                    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom Left
                    -0.5f, 0.5f, 0.0f, 0.0f, 1.0f   // Top Left
                };
                // NOTE(Roman): populate position buffer
                glBindBuffer(GL_ARRAY_BUFFER, gpuItem->quadTexture.vertexBufferObjectId);
                glBufferData(GL_ARRAY_BUFFER, sizeof(fTexQuad), fTexQuad, GL_STATIC_DRAW);
#endif

                if (!targetPtr->gpuPrograms[type].loaded) {
                    if (!targetPtr->gpuPrograms[type].id) {
                        targetPtr->gpuPrograms[type] = NewProgram();
                    }

                    glShader vert = LoadShader(quad::VertexShader01, GL_VERTEX_SHADER);
                    glShader frag = LoadShader(quad::FragmentShader01, GL_FRAGMENT_SHADER);

                    AddShaderToProgram(&targetPtr->gpuPrograms[type], &vert);
                    AddShaderToProgram(&targetPtr->gpuPrograms[type], &frag);

                    // NOTE(Roman): associate the fragment shader output "outputColor" to id=0
                    //  not needed if using only one output, or if "layout (location=0)" is in the shader itself,
                    //  glBindFragDataLocation(targetPtr->gpuPrograms[type].id, 0, "outputColor");

                    LinkProgram(&targetPtr->gpuPrograms[type]);
                }

                gpuItem->quadTexture.programId = targetPtr->gpuPrograms[type].id;
                glUseProgram(gpuItem->quadTexture.programId);

                // NOTE(Roman): query the "in" attributes for their location index,
                // this could be achieved by: 
                //  glBindAttribLocation(gpuItem->quadTexture.programId, 0, "inPosition"); //GLint posAttrib =0
                //  glBindAttribLocation(gpuItem->quadTexture.programId, 1, "inCoord"); //GLint coordAttrib =1
#ifdef SEPARATE_VB
                // NOTE(Roman): was used with two buffers
                GLint posAttrib = glGetAttribLocation(gpuItem->quadTexture.programId, "inPosition");
                glEnableVertexAttribArray(posAttrib);
                glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(real32), (void*)0);
                //
                GLint coordAttrib = glGetAttribLocation(gpuItem->quadTexture.programId, "inCoord");
                glEnableVertexAttribArray(coordAttrib);
                glVertexAttribPointer(coordAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(real32), (void*)0);
#else
                GLint posAttrib = glGetAttribLocation(gpuItem->quadTexture.programId, "inPosition");
                glEnableVertexAttribArray(posAttrib);
                glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*)0);
                //
                GLint coordAttrib = glGetAttribLocation(gpuItem->quadTexture.programId, "inCoord");
                glEnableVertexAttribArray(coordAttrib);
                glVertexAttribPointer(coordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*)sizeof(vec3));
#endif
                GLuint indices[] = {
                    0, 1, 3,
                    1, 2, 3
                };
                glGenBuffers(1, &gpuItem->quadElementsId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuItem->quadElementsId);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

                glBindVertexArray(0);
                hash_map::add<GfxGpuEntity>(&targetPtr->itemMap, *gpuItem, gpuItem->id.Value);
            }

            RenderEntryBitmap* bitmap = (RenderEntryBitmap*)pushRenderElement(targetPtr->renderGroup, RenderEntryBitmap);
            if (NULL != bitmap) {
                RenderGroup *renderGroup = targetPtr->renderGroup;

                bitmap->color = rgba01(1.0f, 1.0f, 1.0f, 1.0f);

                if (renderGroup->renderAlpha > game_math::epsilon) {
                    RenderEntryTransform transform = getRenderEntityTransform(&renderGroup->transform, offset);
                    if (transform.valid) {
                        game_render_engine::GfxTexture *image = GetBitmap(renderGroup->assets, *idPtr);
                        if (image) {
                            RenderEntryBitmap* bitmap = (RenderEntryBitmap*)pushRenderElement(renderGroup, RenderEntryBitmap);
                            if (NULL != bitmap) {
                                bitmap->color.rgba *= renderGroup->renderAlpha;
                                bitmap->transform = transform;
                                bitmap->bitmap = image;
                            }
                        } else {
                            return;
                        }
                    }
                }
                bitmap->transform.p = offset;
            }
        }
        //------------------------------
        void pushTextureQuad(GameRenderTarget* targetPtr, BitmapID* idPtr, const vec3& offset, const vec3& scale, bool32 billboard = false32) {
            gpuItemType type = Type_QuadTexture;
            if (billboard) {
                type = Type_QuadBillboard;
            }

            GfxGpuEntity* gpuItem = NULL;
            GfxTexture tex = {};
            // RenderGroupEntryType_render_entry_bitmap

            if (!hash_map::get(&targetPtr->itemMap, idPtr->Value, gpuItem)) {

                gpuItem = hash_map::add(&targetPtr->itemMap, GfxGpuEntity(), idPtr->Value);
                gpuItem->id = *idPtr;

                gpuItem->type = gpuEntityType_Quad;

                glGenVertexArrays(1, &gpuItem->quadTexture.vertexArrayObject); // NOTE(Roman) : create the VAO
                glBindVertexArray(gpuItem->quadTexture.vertexArrayObject);

#ifdef SEPARATE_VB
                GLuint ids[2];
                glGenBuffers(2, ids);
                gpuItem->quadTexture.vertexBufferObjectId = ids[0];
                gpuItem->quadTexture.textureBufferObjectId = ids[1];

                real32 fQuad[] = {
                    // vertices
                    0.5f, 0.5f, 0.0f,  // Top Right
                    0.5f, -0.5f, 0.0f,  // Bottom Right
                    -0.5f, -0.5f, 0.0f,// Bottom Left
                    -0.5f, 0.5f, 0.0f  // Top Left
                };
                // NOTE(Roman): populate position buffer
                glBindBuffer(GL_ARRAY_BUFFER, gpuItem->quadTexture.vertexBufferObjectId);
                glBufferData(GL_ARRAY_BUFFER, sizeof(fQuad), fQuad, GL_STATIC_DRAW);

                real32 fTex[] = {
                    // texture u,v
                    1.0f, 1.0f,  // Top Right
                    1.0f, 0.0f,  // Bottom Right
                    0.0f, 0.0f,  // Bottom Left
                    0.0f, 1.0f   // Top Left
                };
                // NOTE(Roman): populate texture coords buffer
                glBindBuffer(GL_ARRAY_BUFFER, gpuItem->quadTexture.textureBufferObjectId);
                glBufferData(GL_ARRAY_BUFFER, sizeof(fTex), fTex, GL_STATIC_DRAW);
#else
                glGenBuffers(1, &gpuItem->quadTexture.vertexBufferObjectId);

                TexturedQuadVertex fTexQuad[] = {
                    // vertices
                    0.5f, 0.5f, 0.0f, 1.0f, 1.0f,  // Top Right
                    0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom Right
                    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom Left
                    -0.5f, 0.5f, 0.0f, 0.0f, 1.0f   // Top Left
                };
                // NOTE(Roman): populate position buffer
                glBindBuffer(GL_ARRAY_BUFFER, gpuItem->quadTexture.vertexBufferObjectId);
                glBufferData(GL_ARRAY_BUFFER, sizeof(fTexQuad), fTexQuad, GL_STATIC_DRAW);
#endif

                if (!targetPtr->gpuPrograms[type].loaded) {
                    if (!targetPtr->gpuPrograms[type].id) {
                        targetPtr->gpuPrograms[type] = NewProgram();
                    }

                    glShader vert = LoadShader(quad::VertexShader01, GL_VERTEX_SHADER);
                    glShader frag = LoadShader(quad::FragmentShader01, GL_FRAGMENT_SHADER);

                    AddShaderToProgram(&targetPtr->gpuPrograms[type], &vert);
                    AddShaderToProgram(&targetPtr->gpuPrograms[type], &frag);

                    // NOTE(Roman): associate the fragment shader output "outputColor" to id=0
                    //  not needed if using only one output, or if "layout (location=0)" is in the shader itself,
                    //  glBindFragDataLocation(targetPtr->gpuPrograms[type].id, 0, "outputColor");

                    LinkProgram(&targetPtr->gpuPrograms[type]);
                }

                gpuItem->quadTexture.programId = targetPtr->gpuPrograms[type].id;
                glUseProgram(gpuItem->quadTexture.programId);

                // NOTE(Roman): query the "in" attributes for their location index,
                // this could be achieved by: 
                //  glBindAttribLocation(gpuItem->quadTexture.programId, 0, "inPosition"); //GLint posAttrib =0
                //  glBindAttribLocation(gpuItem->quadTexture.programId, 1, "inCoord"); //GLint coordAttrib =1
#ifdef SEPARATE_VB
                // NOTE(Roman): was used with two buffers
                GLint posAttrib = glGetAttribLocation(gpuItem->quadTexture.programId, "inPosition");
                glEnableVertexAttribArray(posAttrib);
                glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(real32), (void*)0);
                //
                GLint coordAttrib = glGetAttribLocation(gpuItem->quadTexture.programId, "inCoord");
                glEnableVertexAttribArray(coordAttrib);
                glVertexAttribPointer(coordAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(real32), (void*)0);
#else
                GLint posAttrib = glGetAttribLocation(gpuItem->quadTexture.programId, "inPosition");
                glEnableVertexAttribArray(posAttrib);
                glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*)0);
                //
                GLint coordAttrib = glGetAttribLocation(gpuItem->quadTexture.programId, "inCoord");
                glEnableVertexAttribArray(coordAttrib);
                glVertexAttribPointer(coordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*)sizeof(vec3));
#endif
                GLuint indices[] = {
                    0, 1, 3,
                    1, 2, 3
                };
                glGenBuffers(1, &gpuItem->quadElementsId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuItem->quadElementsId);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

                glBindVertexArray(0);
                hash_map::add<GfxGpuEntity>(&targetPtr->itemMap, *gpuItem, gpuItem->id.Value);
            }

            RenderEntryBitmap* bitmap = (RenderEntryBitmap*)pushRenderElement(targetPtr->renderGroup, RenderEntryBitmap);
            if (NULL != bitmap) {
                RenderGroup *renderGroup = targetPtr->renderGroup;

                bitmap->color = rgba01(1.0f, 1.0f, 1.0f, 1.0f);

                if (renderGroup->renderAlpha > game_math::epsilon) {
                    RenderEntryTransform transform = getRenderEntityTransform(&renderGroup->transform, offset);
                    if (transform.valid) {
                        game_render_engine::GfxTexture *image = GetBitmap(renderGroup->assets, *idPtr);
                        if (image) {
                            RenderEntryBitmap* bitmap = (RenderEntryBitmap*)pushRenderElement(renderGroup, RenderEntryBitmap);
                            if (NULL != bitmap) {
                                bitmap->color.rgba *= renderGroup->renderAlpha;
                                bitmap->transform = transform;
                                bitmap->bitmap = image;
                            }
                        } else {
                            return;
                        }
                    }
                }
                bitmap->transform.p = offset;
            }
        }
    }

    rc_macro_cpp void setSamplerId(GLuint programHandle, GLuint sampler) {
        int32 loc = glGetUniformLocation(programHandle, "gSampler");
        if (loc >= 0) {
            glUniform1i(loc, sampler);
        }
    }

    rc_macro_cpp void bindTexture(const GfxTexture& tex, int iTextureUnit, bool sample = false) {
        glActiveTexture(GL_TEXTURE0 + iTextureUnit - 1);
        glBindTexture(GL_TEXTURE_2D, (GLuint)tex.textureId);
        if (sample) {
            glBindSampler(iTextureUnit, (GLuint)tex.sampleId);
        }
    }
    // Utility functions
    bool32 LoadImageToGPU(GfxTexture& tex)
    {
        if (tex.type == textureType_inMemory) {
            if (tex.imageStats.width & (tex.imageStats.width - 1)) {
                Assert(false);
                // Image width is not power of 2.
                return false32;
            }
            if (tex.imageStats.height & (tex.imageStats.height - 1)) {
                Assert(false);
                // Image height is not power of 2.
                return false32;
            }
            glEnable(GL_TEXTURE_2D);

            GLuint iGLId;
            glGenTextures(1, &iGLId);
            tex.textureId = iGLId;
            tex.type = textureType_inGpu;

            // NOTE(Roman): "Bind" the newly created texture : all future texture functions will modify this texture
            bindTexture(tex, iGLId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.imageStats.width, tex.imageStats.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex.data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            /*if (iClamp == 1) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }*/

            // Set minification filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            // Set magnification filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else if (tex.type == textureType_none) {
            return false32;
        }
        return true32;
    }
}

#endif

#endif//_RC_OPENGL_UTIL_H_
