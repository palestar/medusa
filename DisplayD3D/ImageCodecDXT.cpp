/**
	@file ImageCodexDXT.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/3/2010 1:38:43 PM
*/

#include "ImageCodecDXT.h"

//! Define to non-zero to use DirectX surfaces for compression, then fallback to squish if needed..
#define ENABLE_DX_SURFACE_COMPRESSION				1

#if ENABLE_DX_SURFACE_COMPRESSION

#include "ddraw.h"
#include "dxerr9.h"
#include "d3dx9.h"

#include "DisplayDeviceD3D.H"

#pragma comment(lib, "../ThirdParty/DX9/Lib/ddraw.lib" )
#pragma comment(lib, "../ThirdParty/DX9/Lib/dxguid.lib" )
#pragma comment(lib, "../ThirdParty/DX9/Lib/dxerr9.lib" )
#pragma comment(lib, "../ThirdParty/DX9/Lib/d3dx9.lib" )

#endif

// Squish Library include
#include "squish.h"


//---------------------------------------------------------------------------------------------------

#if ENABLE_DX_SURFACE_COMPRESSION

static bool ValidateDD( HRESULT r )
{
	if ( r == DD_OK )
		return true;
	
	LOG_ERROR( "ImageCodecDXT", "Direct Draw Error (0x%x): %s", r, DXGetErrorString9( r ) );
	return false;
}

static IDirect3DDevice9 * GetDevice()
{
	if ( DisplayDeviceD3D::sm_pD3D == NULL )
	{
		if ( (DisplayDeviceD3D::sm_pD3D = Direct3DCreate9( D3D_SDK_VERSION )) == NULL )
			return NULL;
	}

	if ( DisplayDeviceD3D::sm_pD3DD == NULL )
	{
		D3DPRESENT_PARAMETERS d3dpp;
		memset( &d3dpp, 0, sizeof(d3dpp) );
		d3dpp.Windowed = true;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.EnableAutoDepthStencil = true;
		d3dpp.hDeviceWindow = GetDesktopWindow();
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //D3DPRESENT_INTERVAL_DEFAULT;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.FullScreen_RefreshRateInHz = 0;

		dword flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
		if (! ValidateDD( DisplayDeviceD3D::sm_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, flags, &d3dpp, &DisplayDeviceD3D::sm_pD3DD ) ) )
		{
			flags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			flags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

			if (! ValidateDD( DisplayDeviceD3D::sm_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, flags, &d3dpp, &DisplayDeviceD3D::sm_pD3DD ) ) )
				return NULL;
		}
	}

	return DisplayDeviceD3D::sm_pD3DD;
}

static bool EncodeUsingDX( const Buffer & input, Buffer & output, const SizeInt & size, D3DFORMAT format )
{	
	if ( size.width < 4 || size.height < 4 )
		return false;
	IDirect3DDevice9 * pD3DD = GetDevice();
	if ( pD3DD == NULL )
		return false;

	D3DLOCKED_RECT d3dlr;

	IDirect3DSurface9 * pSrc = NULL;
	if ( ValidateDD( pD3DD->CreateOffscreenPlainSurface( size.width, size.height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &pSrc, NULL ) )
		&& ValidateDD( pSrc->LockRect( &d3dlr, NULL, D3DLOCK_DISCARD ) ) )
	{
		// copy pixel data into our source buffer..
		dword dwPitch = size.width * sizeof(Color);
		for(int i=0;i<size.height;++i)
		{
			memcpy( ((byte *)d3dlr.pBits) + (d3dlr.Pitch * i), 
				((byte *)input.buffer()) + (i * dwPitch), dwPitch );
		}
		pSrc->UnlockRect();

		IDirect3DSurface9 * pDst = NULL;
		if ( ValidateDD( pD3DD->CreateOffscreenPlainSurface( size.width, size.height, format, D3DPOOL_DEFAULT, &pDst, NULL ) )
			&& ValidateDD( D3DXLoadSurfaceFromSurface( pDst, NULL, NULL, pSrc, NULL, NULL, D3DX_DEFAULT, 0 ) )
			&& ValidateDD( pDst->LockRect( &d3dlr, NULL, D3DLOCK_READONLY ) ) )
		{
			dword nSize = ((size.width + 3) / 4) * ((size.height + 3) / 4) * (format == D3DFMT_DXT1 ? 8 : 16);
			output.allocate( nSize );
			memcpy( output.buffer(), d3dlr.pBits, nSize );

			// done, release our surfaces and return the compressed texture..
			pDst->UnlockRect();
			pDst->Release();
			pSrc->Release();

			return true;
		}

		if (pDst != NULL )
			pDst->Release();
	}

	if ( pSrc != NULL )
		pSrc->Release();

	return false;
}

static bool DecodeUsingDX( const Buffer & input, Buffer & output, const SizeInt & size, D3DFORMAT format )
{
	if ( size.width < 4 || size.height < 4 )
		return false;
	IDirect3DDevice9 * pD3DD = GetDevice();
	if ( pD3DD == NULL )
		return false;

	D3DLOCKED_RECT d3dlr;

	IDirect3DSurface9 * pSrc = NULL;
	if ( ValidateDD( pD3DD->CreateOffscreenPlainSurface( size.width, size.height, format, D3DPOOL_DEFAULT, &pSrc, NULL ) )
		&& ValidateDD( pSrc->LockRect( &d3dlr, NULL, D3DLOCK_DISCARD ) ) )
	{
		// copy pixel data into our source buffer..
		memcpy( d3dlr.pBits, input.buffer(), input.bufferSize() );
		pSrc->UnlockRect();

		IDirect3DSurface9 * pDst = NULL;
		if ( ValidateDD( pD3DD->CreateOffscreenPlainSurface( size.width, size.height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &pDst, NULL ) )
			&& ValidateDD( D3DXLoadSurfaceFromSurface( pDst, NULL, NULL, pSrc, NULL, NULL, D3DX_DEFAULT, 0 ) )
			&& ValidateDD( pDst->LockRect( &d3dlr, NULL, D3DLOCK_READONLY ) ) )
		{
			dword dwPitch = size.width * sizeof(Color);
			output.allocate( dwPitch * size.height );

			for(int i=0;i<size.height;++i)
			{
				memcpy( ((byte *)output.buffer()) + (i * dwPitch), 
					((byte *)d3dlr.pBits) + (d3dlr.Pitch * i), dwPitch );
			}

			// done, release our surfaces and return the compressed texture..
			pDst->UnlockRect();
			pDst->Release();
			pSrc->Release();

			return true;
		}

		if (pDst != NULL )
			pDst->Release();
	}

	if ( pSrc != NULL )
		pSrc->Release();

	return false;
}

#endif

static void SwapRedBlue( const Buffer & input, Buffer & output )
{
	dword nPixels = input.bufferSize() / sizeof(Color);
	output.allocate( input.bufferSize() );

	Color * pIn = (Color *)input.buffer();
	Color * pOut = (Color *)output.buffer();
	for(dword p=0;p<nPixels;++p)
		pOut[p] = pIn[p].BGRA();
}

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( ImageCodecDXT1, ImageCodec );

int	ImageCodecDXT1::encode( const Buffer & input, Buffer & output, const SizeInt & size, EncodeLevel nLevel )
{
#if ENABLE_DX_SURFACE_COMPRESSION
	if ( EncodeUsingDX( input, output, size, D3DFMT_DXT1 ) )
		return output.bufferSize();
#endif

	Buffer compress;
	SwapRedBlue( input, compress );

	output.allocate( GetStorageRequirements( size.width, size.height, squish::kDxt1 ) );
	squish::CompressImage( (squish::u8 *)compress.buffer(), size.width, size.height, output.buffer(), squish::kDxt1 );
	return output.bufferSize();
}

int ImageCodecDXT1::decode( const Buffer & input, Buffer & output, const SizeInt & size )
{
#if ENABLE_DX_SURFACE_COMPRESSION
	if ( DecodeUsingDX( input, output, size, D3DFMT_DXT1 ) )
		return output.bufferSize();
#endif

	Buffer decompressed;
	decompressed.allocate( size.width * size.height * sizeof(Color) );
	squish::DecompressImage( (squish::u8 *)decompressed.buffer(), size.width, size.height, input.buffer(), squish::kDxt1 );

	SwapRedBlue( decompressed, output );
	return output.bufferSize();
}

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( ImageCodecDXT3, ImageCodec );

int	ImageCodecDXT3::encode( const Buffer & input, Buffer & output, const SizeInt & size, EncodeLevel nLevel )
{
#if ENABLE_DX_SURFACE_COMPRESSION
	if ( EncodeUsingDX( input, output, size, D3DFMT_DXT3 ) )
		return output.bufferSize();
#endif

	Buffer compress;
	SwapRedBlue( input, compress );

	output.allocate( GetStorageRequirements( size.width, size.height, squish::kDxt3 ) );
	squish::CompressImage( (squish::u8 *)compress.buffer(), size.width, size.height, output.buffer(), squish::kDxt3 );
	return output.bufferSize();
}

int ImageCodecDXT3::decode( const Buffer & input, Buffer & output, const SizeInt & size )
{
#if ENABLE_DX_SURFACE_COMPRESSION
	if ( DecodeUsingDX( input, output, size, D3DFMT_DXT3 ) )
		return output.bufferSize();
#endif

	Buffer decompressed;
	decompressed.allocate( size.width * size.height * sizeof(Color) );
	squish::DecompressImage( (squish::u8 *)decompressed.buffer(), size.width, size.height, input.buffer(), squish::kDxt3 );

	SwapRedBlue( decompressed, output );
	return output.bufferSize();
}

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( ImageCodecDXT5, ImageCodec );

int	ImageCodecDXT5::encode( const Buffer & input, Buffer & output, const SizeInt & size, EncodeLevel nLevel )
{
#if ENABLE_DX_SURFACE_COMPRESSION
	if ( EncodeUsingDX( input, output, size, D3DFMT_DXT5 ) )
		return output.bufferSize();
#endif

	Buffer compress;
	SwapRedBlue( input, compress );

	output.allocate( GetStorageRequirements( size.width, size.height, squish::kDxt5 ) );
	squish::CompressImage( (squish::u8 *)compress.buffer(), size.width, size.height, output.buffer(), squish::kDxt5 );
	return output.bufferSize();
}

int ImageCodecDXT5::decode( const Buffer & input, Buffer & output, const SizeInt & size )
{
#if ENABLE_DX_SURFACE_COMPRESSION
	if ( DecodeUsingDX( input, output, size, D3DFMT_DXT5 ) )
		return output.bufferSize();
#endif

	Buffer decompressed;
	decompressed.allocate( size.width * size.height * sizeof(Color) );
	squish::DecompressImage( (squish::u8 *)decompressed.buffer(), size.width, size.height, input.buffer(), squish::kDxt5 );

	SwapRedBlue( decompressed, output );
	return output.bufferSize();
}

//---------------------------------------------------------------------------------------------------
//EOF
