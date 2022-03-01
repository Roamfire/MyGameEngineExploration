/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __GC_2DDx_RENDER_DEVICE_H_
#ifdef __USE_DIRECTX__
// --
#include "../RaidCore/Headers/RaidCore_GameWorld.h"
#include "../RaidCore/Headers/RaidCore_RenderHandler.h"

#include <map>
#pragma comment(lib, "d2d1.lib")

// 2D Direct X Utils
#include <dwrite.h>
#include <wincodec.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dxgi.h>

namespace rc_render
{
    struct gfx_Brush
    {
        gfx_ColorF
            fGfxColor;
        rc_uint16
            iGfxBrushID;
        rc_uint32
            iBrushType;
    };

    struct gfx_TextRender
    {
        gfx_v3
            position;
        gfx_v3
            size;

        rc_uint16
            brushId;
    };
    /******************************************************************
    * Game subsystems resources
    */

    typedef rc_memory_func<ID2D1Brush*> BrushAlloc;
    extern BrushAlloc::_Type*
        m_GfxBrushList;
    extern rc_uint32
        m_GfxBrushCount;

    typedef rc_memory_func<ID2D1Bitmap*> BitmapAlloc;
    extern BitmapAlloc::_Type*
        m_GfxBitmapList;
    extern rc_uint32
        m_GfxBitmapCount;

    struct game_render_target
    {
        ID2D1Factory*
            m_pD2DFactory;
        ID2D1HwndRenderTarget*
            renderTargetPtr;

        D2D1_FACTORY_TYPE
            m_FactoryThreadingMode;
        D2D1_RENDER_TARGET_PROPERTIES
            m_renderTargetProp;
        D2D1_HWND_RENDER_TARGET_PROPERTIES
            m_hWndRenderTargetProp;

        HWND
            m_hWnd;

        RaidCore_GameApp*
            m_pGameWinApp;
    };

    void CreateSolidBrush(game_render_target* TargetPtr, gfx_Brush*, void **);
    void CreateEmptyBitmap(game_render_target* TargetPtr, gfx_Bitmap*, void **);
    void CreateCopyBitmap(game_render_target* TargetPtr, gfx_BitmapResource*, void **);

    void RenderText(game_render_target* TargetPtr, gfx_TextRender*, const char*);
    void RenderLine(game_render_target* TargetPtr, gfx_v3*, gfx_v3*, gfx_Brush*, rc_real32);
    void RenderSquare(game_render_target* TargetPtr, gfx_v3*, gfx_v3*, rc_real32 cornerRadius, gfx_Brush*);
    /******************************************************************
    * Game subsystems resources
    */
    BrushAlloc::_Type*
        m_GfxBrushList;
    rc_uint32
        m_GfxBrushCount;

    BitmapAlloc::_Type*
        m_GfxBitmapList;
    rc_uint32
        m_GfxBitmapCount;

    void Free(game_render_target* TargetPtr)
    {
        for (rc_uint32 i=0; i< m_GfxBrushCount; ++i)
        {
            SafeRelease(& (m_GfxBrushList[i]));
        }
        for (rc_uint32 i=0; i< m_GfxBitmapCount; ++i)
        {
            SafeRelease(& (m_GfxBitmapList[i]));
        }
    }

    std::map<rc_uint32, rc_void_p>
        brushMap;

    void freeRenderTarget(game_render_target* TargetPtr)
    {
        FreeDeviceResources(TargetPtr);
    }

    rc_bool32 Init(game_state* pGameState, rc_void_p pCreateContext, rc_void_p pUserContext)
    {
        game_render_target* TargetPtr = pGameState->renderTargetPtr;

        //TargetPtr->m_renderTargetProp.dpiX = 96.0f;
        //TargetPtr->m_renderTargetProp.dpiY = 96.0f;
        //TargetPtr->m_renderTargetProp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
        //TargetPtr->m_renderTargetProp.pixelFormat.format = DXGI_FORMAT_R32G32B32A32_UINT;
        //TargetPtr->m_renderTargetProp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
        //TargetPtr->m_renderTargetProp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
        //TargetPtr->m_renderTargetProp.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

        TargetPtr->m_renderTargetProp = D2D1::RenderTargetProperties();

        TargetPtr->m_hWndRenderTargetProp.hwnd = NULL;
        TargetPtr->m_hWndRenderTargetProp.pixelSize.width = 0;
        TargetPtr->m_hWndRenderTargetProp.pixelSize.height = 0;
        TargetPtr->m_hWndRenderTargetProp.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

        TargetPtr->m_hWnd = (HWND) pUserContext;
        RECT Rc;
        ::GetClientRect(TargetPtr->m_hWnd, &Rc);
        TargetPtr->m_hWndRenderTargetProp.pixelSize.width = Rc.right-Rc.left;
        TargetPtr->m_hWndRenderTargetProp.pixelSize.height = Rc.bottom-Rc.top;
        TargetPtr->m_hWndRenderTargetProp.hwnd = TargetPtr->m_hWnd;

        TargetPtr->m_FactoryThreadingMode = D2D1_FACTORY_TYPE_MULTI_THREADED;

        TargetPtr->m_pGameWinApp = (RaidCore_GameApp*)pCreateContext;

        if ( ! CreateGlobalResources(TargetPtr) ) 
        {
            return False32;
        }

        if ( ! CreateDeviceResources(TargetPtr)) 
        {
            return False32;
        }
        return True32;
    }

    rc_bool32 CreateGlobalResources(game_render_target* TargetPtr)
    {
        HRESULT hr =  D2D1CreateFactory(TargetPtr->m_FactoryThreadingMode, &(TargetPtr->m_pD2DFactory));

        return hr == S_OK;
    }
    rc_bool32 CreateDeviceResources(game_render_target* TargetPtr)
    {
        HRESULT hr = S_OK;
        if (NULL == TargetPtr->renderTargetPtr)
        {
            hr = TargetPtr->m_pD2DFactory->CreateHwndRenderTarget(
                TargetPtr->m_renderTargetProp,
                TargetPtr->m_hWndRenderTargetProp,
                &TargetPtr->renderTargetPtr
                );
        }
        return hr == S_OK;
    }

    void FreeDeviceResources(game_render_target* TargetPtr)
    {
        std::map<rc_uint32, rc_void_p>::iterator it = brushMap.begin();
        while (brushMap.end() != it) 
        {
            ID2D1Brush * pBrsh = (ID2D1Brush *) it->second;
            SafeRelease(&pBrsh);
            ++it;
        }
        brushMap.clear();
        SafeRelease(&TargetPtr->renderTargetPtr);
    }

    void CreateSolidBrush(game_render_target* TargetPtr, rc_render::gfx_Brush* pGfxBrush, void ** pOutPtrBrush)
    { 
        if (NULL == pOutPtrBrush)
        {
            return;
        }
        // Create a blue brush.

        ID2D1SolidColorBrush* pOut = NULL;
        HRESULT hr = TargetPtr->renderTargetPtr->CreateSolidColorBrush(
            D2D1::ColorF(pGfxBrush->fGfxColor.fRedPercent,pGfxBrush->fGfxColor.fGreenPercent,pGfxBrush->fGfxColor.fBluePercent,pGfxBrush->fGfxColor.fAlphaPercent),
            &pOut
            );

        if (FAILED(hr))
        {
            SafeRelease(&pOut);
        }
        else {
            *pOutPtrBrush = (rc_void_p) pOut;
            pOut = NULL;
        }
    }

    inline D2D1_SIZE_U ConvertRect(gfx_rect& rc)
    {
        D2D1_SIZE_U rcRep;
        rcRep.height = static_cast<UINT32>(rc.bottomRight.y - rc.topLeft.y);
        rcRep.width = static_cast<UINT32>(rc.bottomRight.x - rc.topLeft.x);
        return rcRep;
    }
    inline D2D1_SIZE_U ConvertRect(gfx_v3& rc)
    {
        D2D1_SIZE_U rcRep;
        rcRep.height = static_cast<UINT32>(rc.y);
        rcRep.width = static_cast<UINT32>(rc.x);
        return rcRep;
    }

    void CreateEmptyBitmap(game_render_target* TargetPtr, rc_render::gfx_Bitmap *pGfxBitmap, void ** pOutPtrBitmap)
    {
        if (NULL == pOutPtrBitmap)
        {
            return;
        }
        D2D1_BITMAP_PROPERTIES bmpProp = { { DXGI_FORMAT_R32G32B32A32_UINT ,}, 96.0f, 96.0f };
        D2D1_SIZE_U rcSize = ConvertRect(pGfxBitmap->uRect);

        ID2D1Bitmap* pOut = NULL;
        HRESULT hr = TargetPtr->renderTargetPtr->CreateBitmap(rcSize, bmpProp, & pOut);
        if (FAILED(hr))
        {
            SafeRelease(&pOut);
        }
        else {
            (*pOutPtrBitmap) = pOut;
            pOut = NULL;
        }
    }

    void CreateCopyBitmap(game_render_target* TargetPtr, rc_render::gfx_BitmapResource *pGfxBitmap, void ** pOutPtrBitmap)
    {
        if (NULL == pOutPtrBitmap)
        {
            return;
        }
        D2D1_BITMAP_PROPERTIES bmpProp = { { DXGI_FORMAT_R32G32B32A32_UINT ,}, 96.0f, 96.0f };
        D2D1_SIZE_U rcSize = ConvertRect(pGfxBitmap->bmpSize);

        ID2D1Bitmap* pOut = NULL;
        // TODO(Roman): 
        //   NULL must be replaced with valid memory of image
        HRESULT hr = TargetPtr->renderTargetPtr->CreateBitmap(rcSize, NULL, pGfxBitmap->memoryPitch, bmpProp, & pOut);
        if (FAILED(hr))
        {
            SafeRelease(&pOut);
        }
        else {
            (*pOutPtrBitmap) = pOut;
            pOut = NULL;
        }
    }

    void CreateBitmapFromMemory(game_render_target* TargetPtr, rc_void_p ptr, rc_uint32 length, void ** pOutPtrBitmap)
    {
        if (NULL == pOutPtrBitmap)
        {
            return;
        }
        D2D1_BITMAP_PROPERTIES bmpProp = { { DXGI_FORMAT_R32G32B32A32_UINT ,}, 96.0f, 96.0f };
        BITMAPINFOHEADER * bmpHeader = (BITMAPINFOHEADER*) ptr;
        //bmpHeader->

        D2D1_SIZE_U rcSize ;
        rcSize.width = bmpHeader->biWidth;
        rcSize.height = bmpHeader->biHeight;

        switch (bmpHeader->biBitCount)
        {
        case 24:
            bmpProp.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_SNORM;
            bmpProp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
            break;
        case 32:
            bmpProp.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_SNORM;
            bmpProp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_STRAIGHT;
            break;

        default:
            break;
        }

        ID2D1Bitmap* pOut = NULL;
        // TODO(Roman): 
        //   NULL must be replaced with valid memory of image
        HRESULT hr = TargetPtr->renderTargetPtr->CreateBitmap(rcSize, ((LPSTR)ptr + (WORD)(bmpHeader->biSize)), rcSize.width * bmpHeader->biBitCount / 8, bmpProp, & pOut);
        if (FAILED(hr))
        {
            SafeRelease(&pOut);
        }
        else {
            (*pOutPtrBitmap) = pOut;
            pOut = NULL;
        }
    }

    rc_bool32 startRenderProcess(game_render_target* TargetPtr)
    {
        if (!CreateDeviceResources(TargetPtr))
        {
            return False32;
        }

        TargetPtr->renderTargetPtr->BeginDraw();
        TargetPtr->renderTargetPtr->SetTransform(D2D1::Matrix3x2F::Identity());

        TargetPtr->renderTargetPtr->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f));
        return True32;
    }
    //
    // - completeRender() will finish up, launch the rendering process and flip buffers on the flip chain.
    rc_bool32 completeRender(game_render_target* TargetPtr)
    {
        HRESULT hr = TargetPtr->renderTargetPtr->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            FreeDeviceResources(TargetPtr);
        }

        return SUCCEEDED(hr) ? True32 : False32;
    }

    //
    // Here are the different drawing methods...
    void RenderText(game_render_target* TargetPtr, rc_render::gfx_TextRender*, const char*)
    {
    }

    void RenderBitmap(game_render_target* TargetPtr, rc_render::sys_render_entity*)
    {
    }

    void RenderLine(game_render_target* TargetPtr, gfx_v3* start, gfx_v3* end, rc_render::gfx_Brush* pInBrush, rc_real32 lineThickness)
    {
#if 0
        ID2D1Brush * pBrush = NULL;

        D2D1_POINT_2F * ptSt = (D2D1_POINT_2F*)start;
        D2D1_POINT_2F * ptEd = (D2D1_POINT_2F*)end;

        TargetPtr->renderTargetPtr->DrawLine (*ptSt, *ptEd, pBrush, lineThickness);
#endif
        ID2D1Brush * pBrush = NULL;

        rc_uint32 color = 
            ((static_cast<rc_uint32>(255 * pInBrush->fGfxColor.fRedPercent)) << 16) +
            ((static_cast<rc_uint32>(255 * pInBrush->fGfxColor.fGreenPercent)) << 8) +
            ((static_cast<rc_uint32>(255 * pInBrush->fGfxColor.fBluePercent)) );

        std::map<rc_uint32, rc_void_p>::iterator it = brushMap.find(color);
        if (brushMap.end() == it)
        {
            CreateSolidBrush (TargetPtr, pInBrush, (rc_void_p*) &pBrush);
            if (NULL == pBrush)
            {
                return;
            }
            brushMap.insert( std::map<rc_uint32, rc_void_p>::value_type(color, pBrush) );
        }
        else
        {
            pBrush = (ID2D1Brush *) it->second;
        }

        D2D1_POINT_2F * ptSt = (D2D1_POINT_2F*)start;
        D2D1_POINT_2F * ptEd = (D2D1_POINT_2F*)end;

        TargetPtr->renderTargetPtr->DrawLine (*ptSt, *ptEd, pBrush, lineThickness);
    }

    void RenderPolygon(game_render_target* TargetPtr, gfx_v3*, rc_uint32 ptCount, rc_render::gfx_Brush*)
    {
    }

    void RenderSquare(game_render_target* TargetPtr, gfx_v3* xy, gfx_v3* wh, rc_real32 opacity, rc_render::gfx_Brush* pInBrush)
    {
        ID2D1Brush * pBrush = NULL;

        rc_uint32 color = 
            ((static_cast<rc_uint32>(255 * pInBrush->fGfxColor.fRedPercent)) << 16) +
            ((static_cast<rc_uint32>(255 * pInBrush->fGfxColor.fGreenPercent)) << 8) +
            ((static_cast<rc_uint32>(255 * pInBrush->fGfxColor.fBluePercent)) );

        std::map<rc_uint32, rc_void_p>::iterator it = brushMap.find(color);
        if (brushMap.end() == it)
        {
            CreateSolidBrush (TargetPtr, pInBrush, (rc_void_p*) &pBrush);
            if (NULL == pBrush)
            {
                return;
            }
            brushMap.insert( std::map<rc_uint32, rc_void_p>::value_type(color, pBrush) );
        }
        else
        {
            pBrush = (ID2D1Brush *) it->second;
        }

        D2D1_RECT_F rcSt;
        rcSt.left = xy->x;
        rcSt.top = xy->y;
        rcSt.right = wh->x + xy->x;
        rcSt.bottom = wh->y + xy->y;

        pBrush->SetOpacity(opacity);
        TargetPtr-> renderTargetPtr ->FillRectangle(rcSt, pBrush);
    }
}

#endif
#define __GC_2DDx_RENDER_DEVICE_H_
#endif//__GC_2DDx_RENDER_DEVICE_H_

