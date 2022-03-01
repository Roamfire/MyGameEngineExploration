#ifdef __USE_DIRECTX__

#include "../GameHandler/stdafx.h"
//--------------------------------
#include <dwrite.h>
#include <wincodec.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dxgi.h>

//
// Utility functions

HRESULT LoadImageStorage(const char * resourceName, size_t& imageSize, void ** imagePtr)
{
	return E_FAIL;
}

HRESULT LoadResourceImage(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	const char * resourceName,
	ID2D1Bitmap **ppBitmap)
{
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	size_t imageFileSize=0;
	void * pImageFile = NULL;

	// Create a WIC stream to map onto the memory.
	HRESULT hr = LoadImageStorage (resourceName, imageFileSize, &pImageFile);
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr))
	{
		// Initialize the stream with the memory pointer and size.
		hr = pStream->InitializeFromMemory(
			reinterpret_cast<rc_uint8*>(pImageFile),
			imageFileSize
			);
	}
	if (SUCCEEDED(hr))
	{
		// Create a decoder for the stream.
		hr = pIWICFactory->CreateDecoderFromStream(
			pStream,
			NULL,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
			);
	}
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
			);
	}
	if (SUCCEEDED(hr))
	{
		//create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
			);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}

HRESULT CreateMaskedSprite(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	const char * resourceName,
	ID2D1BitmapBrush **ppBitmapBrush,
	ID2D1BitmapBrush **ppBitmapBrushMask)
{
	ID2D1Bitmap * pOrigImage=NULL;
	ID2D1Bitmap * pOrigImageMask=NULL;
	HRESULT hr = S_OK;

	char * rscMaskName = new char[_tcslen (resourceName) + 4];

	_tcscpy_s(rscMaskName, _tcslen (resourceName) + 4, resourceName);
	_tcscat_s(rscMaskName, _tcslen (resourceName) + 4, _T("Msk"));

	// Create the bitmap to be used by the bitmap brush
	if (SUCCEEDED(hr))
	{
		hr = LoadResourceImage(pRenderTarget,pIWICFactory,resourceName,&pOrigImage);
	}
	if (SUCCEEDED(hr))
	{
		hr = LoadResourceImage(pRenderTarget,pIWICFactory,rscMaskName,&pOrigImageMask);
	}

	if (SUCCEEDED(hr))
	{
		D2D1_BITMAP_BRUSH_PROPERTIES propertiesXClampYClamp = D2D1::BitmapBrushProperties(
			D2D1_EXTEND_MODE_CLAMP,
			D2D1_EXTEND_MODE_CLAMP,
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
			);

		hr = pRenderTarget->CreateBitmapBrush(
			pOrigImage,
			propertiesXClampYClamp,
			ppBitmapBrush
			);

		if (SUCCEEDED(hr))
		{
			hr = pRenderTarget->CreateBitmapBrush(
				pOrigImageMask,
				propertiesXClampYClamp,
				ppBitmapBrushMask
				);
		}
	}

	return hr;
}

void RenderMaskedSprite (
	ID2D1RenderTarget *pRenderTarget,
	D2D1_RECT_F & rcDestRect,
	ID2D1BitmapBrush *pBitmapBrush,
	ID2D1Bitmap *pBitmapMask
	)
{
	
	// D2D1_ANTIALIAS_MODE_ALIASED must be set for FillOpacityMask
	// to function properly.
	pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

	pRenderTarget->FillOpacityMask(
		pBitmapMask,
		pBitmapBrush,
		D2D1_OPACITY_MASK_CONTENT_GRAPHICS,
		&rcDestRect,
		NULL
		);

	pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

#endif