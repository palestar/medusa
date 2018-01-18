/*
DisplayDeviceD3D.cpp
(c)2005 Palestar, Richard Lyle
*/

//#define PROFILE_OFF
#define MEDUSA_TRACE_ON

#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "File/FileDisk.h"
#include "File/Path.h"
#include "Math/Constants.h"
#include "Math/Plane.h"
#include "Math/SphericalHull.h"
#include "Standard/Limits.h"
#include "Draw/ImageCodec.h"

#include "Display/Types.h"
#include "DisplayDeviceD3D.h"
#include "PrimitiveFactory.h"
#include "PrimitiveSurfaceD3D.h"
#include "PrimitiveMaterialD3D.h"
#include "PrimitiveWindowD3D.h"

#include "DisplayEffectHDR.h"
#include "DisplayEffectBlur.h"

#include <math.h>

//---------------------------------------------------------------------------------------------------

LPDIRECT3D9						DisplayDeviceD3D::sm_pD3D = NULL;						// Direct3D object
LPDIRECT3DDEVICE9 				DisplayDeviceD3D::sm_pD3DD = NULL;						// Direct3D Device object
DisplayDeviceD3D::ModeList		DisplayDeviceD3D::sm_ModeList;
DisplayDeviceD3D::DeviceList	DisplayDeviceD3D::sm_DeviceList;

//---------------------------------------------------------------------------------------------------

//! How far out from the eye to clip the light position for shadow map generation
const float DEFAULT_SHADOW_RADIUS = 1500.0f;
//! How big are the shadow maps by default.
const int	DEFAULY_SHADOW_MAP_SIZE = 2048;

//---------------------------------------------------------------------------------------------------

static const int VERTEX_SIZES[DisplayDeviceD3D::VB_COUNT] =
{
	sizeof(Vertex),
	sizeof(VertexL),
	sizeof(VertexTL)
};

static const dword VERTEX_FVF[DisplayDeviceD3D::VB_COUNT] =
{
	D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1,						// Vertex
	D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1,		// VertexL
	D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1					// VertexTL
};

IMPLEMENT_RAW_STREAMING(D3DXMATRIX);
IMPLEMENT_RAW_STREAMING(D3DXVECTOR4);
IMPLEMENT_RAW_STREAMING(D3DVECTOR);
IMPLEMENT_RAW_STREAMING(D3DCOLORVALUE);

//---------------------------------------------------------------------------------------------------

IMPLEMENT_FACTORY(DisplayDeviceD3D, DisplayDevice);

DisplayDeviceD3D::DisplayDeviceD3D() :
	m_dwLockingThread(0),
	m_nLockCount(0),
	m_bBeginScene(false),
	m_nTextureStage(0),
	m_TextureP2(true),
	m_TextureSquare(false),
	m_TextureMaxSize(256, 256),
	m_TextureMinSize(8, 8),
	m_fPixelShaderVersion(0.0f),
	m_fVertexShaderVersion(0.0f),
	m_eFilterMode(FILTER_ANISOTROPIC),
	m_HWND(NULL),
	m_bHardware(true),
	m_bWindowed(false),
	m_eFSAA(FSAA_NONE),
	m_ClientRectangle(0, 0, -1, -1),
	m_bMinimized(false),
	m_pD3D(NULL),
	m_pD3DD(NULL),
	m_pD3DSC(NULL),
	m_pBackBuffer(NULL),
	m_cAmbientLight(BLACK),
	m_NextLightId(0),
	m_pVS(NULL),
	m_pVSConstants(NULL),
	m_pPS(NULL),
	m_pPSConstants(NULL),
	m_bShadowMapReady(false),
	m_bShadowMapSupported(true),
	m_bShadowPass(false),
	m_nMaxShadowLights(0),
	m_nShadowMapPass(0),
	m_bUsingFixedFunction(false),
	m_vShadowFocus(0, 0, 1.0f),
	m_fShadowRadius(DEFAULT_SHADOW_RADIUS),
	m_szShadowMap(DEFAULY_SHADOW_MAP_SIZE, DEFAULY_SHADOW_MAP_SIZE),
	m_pShadowMapDS(NULL)
{
	TRACE("DisplayDeviceD3D created!");

	lock();

	for (int vb = 0; vb<VB_COUNT; vb++)
	{
		m_VB[vb] = NULL;
		m_VBOffset[vb] = 0;
	}

	// have all devices / modes been enumerated?
	enumerateModes();

	// register our supported effects
	registerEffect("HDR", DisplayEffectHDR::staticFactory());
	registerEffect("BLUR", DisplayEffectBlur::staticFactory());

	// declare D3D types so Value can hold and copy the data..
	TypeSerialized<D3DXMATRIX>::instance();
	TypeCopyDefault<D3DXMATRIX, D3DXMATRIX>::instance();

	TypeSerialized<D3DVECTOR>::instance();
	TypeCopyDefault<D3DVECTOR, D3DVECTOR>::instance();

	TypeSerialized<D3DXVECTOR4>::instance();
	TypeCopyDefault<D3DXVECTOR4, D3DXVECTOR4>::instance();

	TypeSerialized<D3DCOLORVALUE>::instance();
	TypeCopyDefault<D3DCOLORVALUE, D3DCOLORVALUE>::instance();

	unlock();
}

DisplayDeviceD3D::~DisplayDeviceD3D()
{
	TRACE("DisplayDeviceD3D destroyed!");

	// release this device
	release();
}

//------------------------------------------------------------------------------------------------

bool DisplayDeviceD3D::isLocked() const
{
	return ::GetCurrentThreadId() == m_dwLockingThread;
}

int DisplayDeviceD3D::modeCount() const
{
	return sm_ModeList.size();
}

const DisplayDevice::Mode * DisplayDeviceD3D::mode(int n) const
{
	return &sm_ModeList[n];
}

ColorFormat * DisplayDeviceD3D::primaryFormat() const
{
	return m_pFormat;
}

int DisplayDeviceD3D::surfaceFormatCount() const
{
	return m_TextureFormats.size();
}

ColorFormat::Format DisplayDeviceD3D::surfaceFormat(int n) const
{
	return m_TextureFormats[n];
}

const DisplayDevice::Mode * DisplayDeviceD3D::activeMode() const
{
	return &m_Mode;
}

bool DisplayDeviceD3D::windowed() const
{
	return m_bWindowed;
}

RectInt DisplayDeviceD3D::clientWindow() const
{
	return m_ClientRectangle;
}

RectInt DisplayDeviceD3D::renderWindow() const
{
	D3DVIEWPORT9 d3dvp;
	m_pD3DD->GetViewport(&d3dvp);

	return RectInt(PointInt(d3dvp.X, d3dvp.Y), SizeInt(d3dvp.Width, d3dvp.Height));
}

bool DisplayDeviceD3D::textureP2() const
{
	return m_TextureP2;
}

bool DisplayDeviceD3D::textureSquare() const
{
	return m_TextureSquare;
}

SizeInt DisplayDeviceD3D::textureMaxSize() const
{
	return m_TextureMaxSize;
}

SizeInt DisplayDeviceD3D::textureMinSize() const
{
	return m_TextureMinSize;
}

float DisplayDeviceD3D::pixelShaderVersion() const
{
	return m_fPixelShaderVersion;
}

float DisplayDeviceD3D::vertexShaderVersion() const
{
	return m_fVertexShaderVersion;
}

DisplayDevice::FilterMode DisplayDeviceD3D::filterMode() const
{
	return m_eFilterMode;
}

dword DisplayDeviceD3D::totalVideoMemory() const
{
	return m_pD3DD->GetAvailableTextureMem();
}

dword DisplayDeviceD3D::freeVideoMemory() const
{
	return m_pD3DD->GetAvailableTextureMem();
}

//------------------------------------------------------------------------------------------------

void DisplayDeviceD3D::lock()
{
	m_Lock.lock();
	if (m_nLockCount == 0)
		m_dwLockingThread = ::GetCurrentThreadId();
	++m_nLockCount;
}

void DisplayDeviceD3D::unlock()
{
	if (--m_nLockCount <= 0)
		m_dwLockingThread = 0;
	m_Lock.unlock();
}

bool DisplayDeviceD3D::initialize(void *hWnd, const Mode * pMode, bool bWindowed, bool bHardware, FSAA eFSAA /*= FSAA_NONE*/)
{
	if (pMode == NULL && !bWindowed)
		return false;

	lock();
	// release the device if being reinitialized
	release();

	// save the window handle
	m_HWND = static_cast<HWND>(hWnd);
	m_bHardware = bHardware;
	m_eFSAA = eFSAA;

	// set the mode 
	if (pMode != NULL)
	{
		m_Mode = *pMode;
		m_pFormat = ColorFormat::allocateFormat(pMode->colorFormat);
	}
	else
	{
		if (!bWindowed)
		{
			TRACE("Mode required for non-windowed modes!");
			return false;
		}

		m_pFormat = ColorFormat::allocateFormat(ColorFormat::RGB888);
	}

	m_bWindowed = bWindowed;

	// get the starting window placement so we can restore it..
	m_ClientPlacement.length = sizeof(m_ClientPlacement);
	GetWindowPlacement(m_HWND, &m_ClientPlacement);

	// Are we creating the first RenderDevice using this hardware?
	if (!initializeDirectX())
	{
		unlock();
		return false;
	}

	// add to device list
	sm_DeviceList.push(this);
	unlock();

	return true;
}

bool DisplayDeviceD3D::setMode(const Mode * pMode, bool bWindowed)
{
	if (pMode == NULL && !bWindowed)
		return false;

	if (pMode != NULL)
		m_Mode = *pMode;
	m_bWindowed = bWindowed;

	if (!resetDirectX())
		return false;

	if (m_bWindowed)
	{
		int cx = m_Mode.screenSize.width + (GetSystemMetrics(SM_CXSIZEFRAME) * 2);
		int cy = m_Mode.screenSize.height + (GetSystemMetrics(SM_CYSIZEFRAME) * 2)
			+ GetSystemMetrics(SM_CYCAPTION);

		::SetWindowPos(m_HWND, HWND_NOTOPMOST, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOZORDER);
	}

	return true;
}

void DisplayDeviceD3D::setFilterMode(FilterMode eMode)
{
	m_eFilterMode = eMode;
}

void DisplayDeviceD3D::release()
{
	lock();
	// remove from device list
	sm_DeviceList.removeSearch(this);
	// release the primitives
	abortScene();
	// release the texture formats
	m_TextureFormats.release();

	// release the format
	m_pFormat = NULL;

	freeDirectX();
	unlock();
}

void DisplayDeviceD3D::flushVideoMemory() const
{
	// previously we called EvictManagedTextures()!
}

void DisplayDeviceD3D::clear(Color nColor)
{
	validateRenderTarget();
	if (m_pD3DD->Clear(0, NULL, D3DCLEAR_TARGET, nColor.BGRA(), 0, 0) != D3D_OK)
	{
		TRACE("DisplayDeviceD3D::clear() - Clear failed!");
	}
}

void DisplayDeviceD3D::clearZ(float fDepth /*= 1.0f*/)
{
	validateRenderTarget();
	if (m_pD3DD->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, fDepth, 0) != D3D_OK)
	{
		TRACE("DisplayDeviceD3D::clearZ() - Clear failed!");
	}
}

void DisplayDeviceD3D::setAmbient(Color nColor)
{
	m_cAmbientLight = nColor;
}

int DisplayDeviceD3D::addDirectionalLight(int nPriority, Color nColor, const Vector3 & vDirection)
{
	if (m_bShadowPass)
		return -1;

	// setup the light
	D3DLIGHT9 light;
	memset(&light, 0, sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;

	light.Direction.x = vDirection.x;
	light.Direction.y = vDirection.y;
	light.Direction.z = vDirection.z;

	const float resp = 1.0f / 255.0f;
	light.Specular.r = light.Diffuse.r = nColor.r * resp;
	light.Specular.g = light.Diffuse.g = nColor.g * resp;
	light.Specular.b = light.Diffuse.b = nColor.b * resp;
	light.Specular.a = light.Diffuse.a = 1.0f; //nColor.a * resp;

	dword lightIndex = m_NextLightId++;
	m_pD3DD->SetLight(lightIndex, &light);
	m_pD3DD->LightEnable(lightIndex, true);

	m_Lights.insert(std::pair<int, D3DLIGHT9>(nPriority, light));
	return m_Lights.size();
}

int DisplayDeviceD3D::addPointLight(int nPriority, Color nColor, const Vector3 & vPosition, float fRadius)
{
	if (m_bShadowPass)
		return -1;

	// setup the light
	D3DLIGHT9 light;
	memset(&light, 0, sizeof(light));
	light.Type = D3DLIGHT_POINT;

	const float resp = 1.0f / 255.0f;
	light.Specular.r = light.Diffuse.r = nColor.r * resp;
	light.Specular.g = light.Diffuse.g = nColor.g * resp;
	light.Specular.b = light.Diffuse.b = nColor.b * resp;
	light.Specular.a = light.Diffuse.a = 1.0f; //nColor.a * resp;

	light.Position.x = vPosition.x;
	light.Position.y = vPosition.y;
	light.Position.z = vPosition.z;

	light.Range = fRadius;
	light.Attenuation0 = 1.0f;
	if (light.Range > 0.0f)
		light.Attenuation1 = 1.0f / light.Range; //sqrt( light.Range ) / light.Range;
	else
		light.Attenuation1 = 0.0f;
	light.Attenuation2 = 0.0f;

	m_Lights.insert(std::pair<int, D3DLIGHT9>(nPriority, light));
	return m_Lights.size();
}

void DisplayDeviceD3D::clearLights()
{
	m_Lights.clear();
	m_NextLightId = 0;
}

void DisplayDeviceD3D::setFillMode(FillMode nMode)
{
	switch (nMode)
	{
	case FILL_SOLID:
		m_pD3DD->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		break;
	case FILL_WIREFRAME:
		m_pD3DD->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		break;
	case FILL_POINT:
		m_pD3DD->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
		break;
	}
}

void DisplayDeviceD3D::setFog(FogMode nMode, float fBegin, float fEnd, Color nColor)
{
	if (nMode != FOG_NONE)
	{
		static dword FOG_LOOKUP[] =
		{
			D3DFOG_NONE,	// FOG_NONE,
			D3DFOG_EXP,		// FOG_EXP,
			D3DFOG_EXP2,	// FOG_EXP2,
			D3DFOG_LINEAR,	// FOG_LINEAR,
		};

		if (m_pD3DD->SetRenderState(D3DRS_FOGENABLE, TRUE) != D3D_OK ||
			m_pD3DD->SetRenderState(D3DRS_FOGCOLOR, nColor.BGRA()) != D3D_OK ||
			m_pD3DD->SetRenderState(D3DRS_FOGVERTEXMODE, FOG_LOOKUP[nMode]) != D3D_OK ||
			m_pD3DD->SetRenderState(D3DRS_FOGSTART, *((dword *)&fBegin)) != D3D_OK ||
			m_pD3DD->SetRenderState(D3DRS_FOGEND, *((dword *)&fEnd)) != D3D_OK)
		{
			TRACE("DisplayDeviceD3D::setFog() - Failed to set the fog!");
		}
	}
	else
		m_pD3DD->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

void DisplayDeviceD3D::disableFog()
{
	m_pD3DD->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

void DisplayDeviceD3D::setProjection(const Matrix33 & mFrame, const Vector3 & vPosition,
	const RectInt & rWindow, float fFOV, float fFront, float fBack)
{
	m_Proj.m_mFrame = mFrame;
	m_Proj.m_vPosition = vPosition;
	m_Proj.m_rWindow = rWindow;
	m_Proj.m_fFOV = fFOV;
	m_Proj.m_fFront = fFront;
	m_Proj.m_fBack = fBack;

	updateProjection();
}

void DisplayDeviceD3D::setShadowPass(int a_nMaxLights, const Vector3 & a_vFocus,
	float a_fShadowRadius, const SizeInt & a_szShadowMap)
{
	m_nMaxShadowLights = a_nMaxLights;
	m_vShadowFocus = a_vFocus;
	m_fShadowRadius = a_fShadowRadius;

	if (m_szShadowMap != a_szShadowMap)
	{
		m_szShadowMap = a_szShadowMap;

		// release our shadow maps since the size has changed...
		m_ShadowPassList.clear();			// the ShadowPass() destructor will release all shadow map textures
		m_bShadowMapReady = false;
	}
}

//---------------------------------------------------------------------------------------------------

bool DisplayDeviceD3D::beginScene()
{
	m_bFirstShadowPass = true;
	m_nShadowMapPass = 0;
	m_pCurrentTransform = NULL;
	m_pCurrentMaterial = NULL;

	validateRenderTarget();
	return true;
}

bool DisplayDeviceD3D::beginShadowPass(Transform & a_LightTransform)
{
	if (!m_bShadowMapSupported)
		return false;			// shadow maps not supported
	if (m_bShadowPass)
		return false;			// beginShadowPass() already been called..

								// set up iterators on first pass..
	if (m_bFirstShadowPass)
	{
		m_bFirstShadowPass = false;
		m_ShadowPassList.resize(Min<int>(m_nMaxShadowLights, m_Lights.size()));
		m_iCurrentShadowPass = m_ShadowPassList.begin();
		m_iCurrentShadowLight = m_Lights.begin();
	}

	if (m_iCurrentShadowLight == m_Lights.end())
		return false;
	if (m_iCurrentShadowPass == m_ShadowPassList.end())
		return false;

	// do one-time initialization if needed..
	if (!readyShadowMap())
		return false;

	D3DLIGHT9 & light = m_iCurrentShadowLight->second;

	// get the lights position in world space
	Vector3 vLightViewPosition(Vector3::ZERO);
	Vector3 vLightViewDirection(Vector3::ZERO);

	float fMidZ = (m_Proj.m_fBack - m_Proj.m_fFront) * 0.5f;
	Vector3 vWorldFocus(m_Proj.m_vPosition + (m_Proj.m_mFrame % m_vShadowFocus));		// transform to world space

	float fShadowScale = 0.0f;

	bool bOrthoProj = false;
	if (light.Type == D3DLIGHT_DIRECTIONAL)
	{
		vLightViewDirection = Vector3(light.Direction.x, light.Direction.y, light.Direction.z);
		vLightViewPosition = vWorldFocus - (vLightViewDirection * fMidZ);
		fShadowScale = light.Diffuse.r * light.Diffuse.g * light.Diffuse.b;

		bOrthoProj = true;
	}
	else if (light.Type == D3DLIGHT_POINT)
	{
		vLightViewPosition = Vector3(light.Position.x, light.Position.y, light.Position.z);
		vLightViewDirection = vWorldFocus - vLightViewPosition;
		float fDistance = vLightViewDirection.magnitude();
		vLightViewDirection *= 1.0f / fDistance;		// normalize the vector

		if (fDistance < light.Range)
		{
			if (fDistance > 0.0f)
				fShadowScale = 1.0f - (fDistance / light.Range);
			else
				fShadowScale = 1.0f;
		}
	}

	// don't shadow if light isn't bright enought to cast a shadow..
	if (fShadowScale <= 0.05f)
		return false;

	// clip the light to our shadow focus area..
	SphericalHull sphere(m_vShadowFocus, m_fShadowRadius);
	// transform light position to view space
	Vector3 vClippedLightPos(m_Proj.m_mFrame * (vLightViewPosition - m_Proj.m_vPosition));
	// intersect with a sphere around our focus point...
	sphere.intersect(vClippedLightPos, m_vShadowFocus - vClippedLightPos, vClippedLightPos);
	// transform light position back to world space..
	vLightViewPosition = m_Proj.m_vPosition + (m_Proj.m_mFrame % vClippedLightPos);

	ShadowPass & pass = *m_iCurrentShadowPass;
	pass.m_LightTransform.m_mFrame = Matrix33(vLightViewDirection);
	pass.m_LightTransform.m_vTranslate = vLightViewPosition;
	pass.m_bOrthoProj = bOrthoProj;
	pass.m_Primitives.release();

	a_LightTransform = pass.m_LightTransform;
	m_bShadowPass = true;
	m_pCurrentTransform = NULL;
	m_pCurrentMaterial = NULL;

	return true;
}

bool DisplayDeviceD3D::endShadowPass()
{
	if (!m_bShadowPass)
		return false;

	m_bShadowPass = false;

	IDirect3DStateBlock9 * pStateBlock = NULL;
	if (m_pD3DD->CreateStateBlock(D3DSBT_ALL, &pStateBlock) != D3D_OK)
	{
		TRACE("Failed to create state block!");
		return false;
	}

	// get the current render target
	IDirect3DSurface9 * pRenderTarget = NULL;
	if (m_pD3DD->GetRenderTarget(0, &pRenderTarget) != D3D_OK)
	{
		TRACE("Failed to get render target!");
		return false;
	}

	// save the current projection so it can be restored after rendering the shadow map
	Projection savedProjection(m_Proj);

#if ENABLE_SHADOW_MAP_DS
	// change the depth surface to the one used for our shadow maps
	IDirect3DSurface9 * pPrimaryDS = NULL;
	if (m_pD3DD->GetDepthStencilSurface(&pPrimaryDS) != D3D_OK)
		TRACE("Failed to get depth surface!");
	m_pD3DD->SetDepthStencilSurface(m_pShadowMapDS);
#endif

	ShadowPass & pass = *m_iCurrentShadowPass;
	if (pass.m_pShadowMap == NULL)
	{
		if (m_pD3DD->CreateTexture(
			m_szShadowMap.width,
			m_szShadowMap.height,
			1, D3DUSAGE_RENDERTARGET, /*D3DFMT_A8R8G8B8*/D3DFMT_R32F, D3DPOOL_DEFAULT,
			&pass.m_pShadowMap, NULL) != D3D_OK)
		{
			TRACE("Failed to create shadow map!");
			return false;
		}
	}

	IDirect3DSurface9 * pShadowMapSur = NULL;
	if (pass.m_pShadowMap->GetSurfaceLevel(0, &pShadowMapSur) != D3D_OK)
	{
		TRACE("Failed to get shadow map surface");
		return false;
	}
	// set our render target to the shadow map
	if (m_pD3DD->SetRenderTarget(0, pShadowMapSur) != D3D_OK)
	{
		TRACE("Failed to set render target surface!");
		return false;
	}

	// set the projection matrix for our light
	m_Proj.m_rWindow = RectInt(PointInt(0, 0), m_szShadowMap);
	m_Proj.m_mFrame = pass.m_LightTransform.m_mFrame;
	m_Proj.m_vPosition = pass.m_LightTransform.m_vTranslate;
	updateProjection();

	// Get the view transform from the device, set by setProjection() above.. we need to pass it into our shader
	m_pD3DD->GetTransform(D3DTS_VIEW, &pass.m_LightView);
	m_pD3DD->GetTransform(D3DTS_PROJECTION, &pass.m_LightProj);

	m_pShadowMap->setConstant("mLightView", pass.m_LightView);
	m_pShadowMap->apply(this);

	// clear the color & z-buffer for each shadow pass..
	float fZ = m_Proj.m_fBack;
	m_pD3DD->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, *((DWORD *)&fZ), 1.0f, 0);

	// set our states..
	m_pD3DD->SetTexture(0, NULL);
	m_pD3DD->SetTexture(1, NULL);
	m_pD3DD->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pD3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);		// render back-faces only..
																// render our primary geometry from the lights perspective to generate our shadow map
	for (int i = 0; i<pass.m_Primitives.size(); ++i)
		pass.m_Primitives[i]->execute();
	pass.m_Primitives.release();

	if (pRenderTarget != NULL)
	{
		m_pD3DD->SetRenderTarget(0, pRenderTarget);
		RELEASEDD(pRenderTarget);
	}
	if (pStateBlock != NULL)
	{
		pStateBlock->Apply();
		RELEASEDD(pStateBlock);
	}

	RELEASEDD(pShadowMapSur);

	// restored projection matrix & view..
	m_Proj = savedProjection;
	updateProjection();

	// restore/clear the z-buffer
#if ENABLE_SHADOW_MAP_DS
	if (pPrimaryDS != NULL)
	{
		m_pD3DD->SetDepthStencilSurface(pPrimaryDS);
		RELEASEDD(pPrimaryDS);
	}
#else
	m_pD3DD->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
#endif

	// increment the shadow map pass..
	++m_iCurrentShadowPass;
	++m_iCurrentShadowLight;
	++m_nShadowMapPass;

	return true;
}

bool DisplayDeviceD3D::endScene()
{
	if (m_bMinimized || m_pD3DD->TestCooperativeLevel() != D3D_OK)
	{
		abortScene();
		return false;
	}

	validateRenderTarget();

	// setup effects first..
	for (EffectList::iterator iEffect = m_EffectList.begin();
		iEffect != m_EffectList.end(); )
	{
		DisplayEffect * pEffect = *iEffect;
		if (!pEffect->preRender(this))
		{
			TRACE("ERROR: Effect preRender() failed.");
			m_EffectList.erase(iEffect++);	// remove from the list now, we won't bother calling postRender() then..
		}
		else
			++iEffect;
	}

	// setup the lights even if we are not using fixed function, because we may fallback to fixed function for certain materials..
	if (m_Lights.size() > 0)
	{
		// enable lighting...
		m_pD3DD->SetRenderState(D3DRS_LIGHTING, TRUE);
		m_pD3DD->SetRenderState(D3DRS_AMBIENT, m_cAmbientLight.BGRA());

		// our multi-map has our lights sorted in order of priroity, initialize our lights starting with the most important
		// lights, then continue until we run out of lights or we hit the MAX_LIGHTS
		LightMap::iterator iLight = m_Lights.begin();

#if defined(_DEBUG)
		int nLastPriority = 0x7fffffff;
#endif
		for (int i = 0; i<PrimitiveMaterial::sm_nMaxLights; ++i)
		{
			if (iLight != m_Lights.end())
			{
				D3DLIGHT9 & light = iLight->second;
#if defined(_DEBUG)
				ASSERT(iLight->first <= nLastPriority);
				nLastPriority = iLight->first;
#endif
				++iLight;

				m_pD3DD->SetLight(i, &light);
				m_pD3DD->LightEnable(i, true);
			}
			else
			{
				m_pD3DD->LightEnable(i, false);
			}
		}
	}
	else
	{
		// no lights, turn off lighting
		m_pD3DD->SetRenderState(D3DRS_LIGHTING, FALSE);
	}

	m_pD3DD->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);				// enable the z buffer
	m_pD3DD->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);				// set the default culling mode
	m_pD3DD->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE);			// write enabled for primary render pass
	m_pD3DD->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);			// set the default z comparison mode

	for (int i = 0; i<PASS_COUNT; ++i)
	{
		Array< PrimitiveMaterial::Ref > & materials = m_Stack[i];
		for (int j = 0; j<materials.size(); ++j)
		{
			PrimitiveMaterial * pMaterial = materials[j];
			pMaterial->execute();
			pMaterial->clear();
		}
		materials.release();
	}

	// for effects to chain, we must process them in reverse order..
	for (EffectList::reverse_iterator iEffect = m_EffectList.rbegin();
		iEffect != m_EffectList.rend(); ++iEffect)
	{
		DisplayEffect * pEffect = *iEffect;
		if (!pEffect->postRender(this))
			TRACE("ERROR: Effect postRender() returned false!");
	}
	m_EffectList.clear();

	return true;
}

void DisplayDeviceD3D::abortScene()
{
	// release all primitives
	for (int i = 0; i<PASS_COUNT; i++)
	{
		Array< PrimitiveMaterial::Ref > & materials = m_Stack[i];
		for (int j = 0; j<materials.size(); j++)
			materials[j]->clear();
		materials.release();
	}
}

void DisplayDeviceD3D::present()
{
	PROFILE_START("DisplayDeviceD3D::present");

	if (m_bBeginScene)
	{
		m_pD3DD->EndScene();
		m_bBeginScene = false;
	}

	if (m_pD3DSC != NULL)
		m_pD3DSC->Present(NULL, NULL, NULL, NULL, 0);
	else
		m_pD3DD->Present(NULL, NULL, NULL, NULL);

	// check the client window
	updateClientArea(true);
	// if the device is lost, test to see if we should restore the lost device
	if (m_pD3DD->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		resetDirectX();

	PROFILE_END();
}

DevicePrimitive * DisplayDeviceD3D::create(const PrimitiveKey &key)
{
	BasePrimitiveFactory *pFactory = BasePrimitiveFactory::findFactory(key);
	if (pFactory == NULL)
		throw PrimitiveFailure();
	DevicePrimitive * pPrimitive = pFactory->create();
	if (pPrimitive == NULL)
		throw PrimitiveFailure();

	// set the display device
	pPrimitive->setDevice(this);
	// return the created primitive!
	return pPrimitive;
}

bool DisplayDeviceD3D::push(DevicePrimitive * pPrimitive)
{
	if (pPrimitive == NULL)
		return false;

	pPrimitive->setDevice(this);

	if (pPrimitive->primitiveKey() == PrimitiveMaterial::staticPrimitiveKey())
	{
		m_pCurrentMaterial = (PrimitiveMaterial *)pPrimitive;

		if (!m_bShadowPass)
		{
			PrimitiveMaterialD3D * pMaterial = (PrimitiveMaterialD3D *)pPrimitive;
			if (!pMaterial->m_bPushed)
			{
				// push this material into the stack
				m_Stack[pMaterial->m_nPass].push(pMaterial);
				pMaterial->m_bPushed = true;
			}

			// push the current transform as a child
			if (m_pCurrentTransform.valid())
				pMaterial->addChild(m_pCurrentTransform);
		}
		else
		{
			ShadowPass & pass = *m_iCurrentShadowPass;
			if (m_pCurrentTransform.valid() && (pass.m_Primitives.size() == 0 || pass.m_Primitives.last() != m_pCurrentTransform))
				pass.m_Primitives.push((DevicePrimitive *)m_pCurrentTransform);
		}
	}
	else
	{
		if (pPrimitive->primitiveKey() == PrimitiveSetTransform::staticPrimitiveKey())
			m_pCurrentTransform = (PrimitiveSetTransform *)pPrimitive;

		if (!m_bShadowPass)
		{
			// attach the primitive to the current material
			if (m_pCurrentMaterial.valid())
				m_pCurrentMaterial->addChild(pPrimitive);
		}
		else
		{
			// shadow pass, only push PRIMARY primitives into the shadow pass structure..
			ShadowPass & pass = *m_iCurrentShadowPass;
			if (!m_pCurrentMaterial.valid() || m_pCurrentMaterial->pass() == PRIMARY)
				pass.m_Primitives.push(pPrimitive);
		}
	}

	return true;
}

DisplayEffect::Ref DisplayDeviceD3D::createEffect(const char * pName)
{
	EffectMap::iterator iFactory = m_EffectMap.find(pName);
	if (iFactory == m_EffectMap.end())
		return NULL;

	Factory * pFactory = iFactory->second;
	if (!pFactory)
		return NULL;

	Widget::Ref pUncasted = pFactory->createWidget();
	if (!pUncasted)
		return NULL;
	DisplayEffect::Ref pEffect = WidgetCast<DisplayEffect>(pUncasted);
	if (!pEffect.valid())
		return NULL;

	// add to list of created effects, so if the directX device needs to be reset we can
	// release any of their DirectX resources..
	m_CreatedEffects.push_back(pEffect.pointer());

	return pEffect;
}

bool DisplayDeviceD3D::push(DisplayEffect * pEffect)
{
	m_EffectList.push_back(pEffect);
	return true;
}

//----------------------------------------------------------------------------

#pragma pack(push, DISPLAYDEVICED3D )
#pragma pack(1)

struct RGB
{
	byte	b, g, r;
};

struct TGA_HEADER
{
	byte	identsize;          // size of ID field that follows 18 byte header (0 usually)
	byte	palette;			// type of color map 0=none, 1=has palette
	byte	imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

	word	color_start;		// first color map entry in palette
	word	num_palette_colors;	// number of colors in palette
	byte	bits_per_color;		// number of bits per palette entry 15,16,24,32

	word	xstart;             // image x origin
	word	ystart;             // image y origin
	word	width;              // image width in pixels
	word	height;             // image height in pixels
	byte	bits_per_pixel;		// image bits per pixel 8,16,24,32
	byte	descriptor;			// image descriptor bits (vh flip bits)

};

#define TGA_DESC_FLIPX			(0x10)
#define TGA_DESC_FLIPY			(0x20)

enum	//  TGA_TYPE
{
	TGA_NONE = 0,
	TGA_INDEXED_UNCOMPRESSED = 1,
	TGA_RGB_UNCOMPRESSED = 2,
	TGA_GREY_UNCOMPRESSED = 3,
	TGA_INDEXED_COMPRESSED = 9,
	TGA_RGB_COMPRESSED = 10,
	TGA_GREY_COMPRESSED = 11,
};

#pragma pack( pop, DISPLAYDEVICED3D )

bool DisplayDeviceD3D::capture(const char * pFilename)
{
	D3DDISPLAYMODE d3ddm;
	m_pD3DD->GetDisplayMode(0, &d3ddm);

	// create a surface to capture the front buffer
	LPDIRECT3DSURFACE9 primary;
	if (m_pD3DD->CreateOffscreenPlainSurface(d3ddm.Width, d3ddm.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &primary, NULL) != D3D_OK)
		return false;
	// copy the front buffer into our surface
	if (m_pD3DD->GetFrontBufferData(0, primary) != D3D_OK)
		return false;

	D3DSURFACE_DESC ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	if (primary->GetDesc(&ddsd) != D3D_OK)
		return false;

	// lock the primary buffer
	D3DLOCKED_RECT lock;
	memset(&lock, 0, sizeof(lock));
	if (primary->LockRect(&lock, NULL, D3DLOCK_READONLY))
		return false;

	Path p(pFilename);
	if (stricmp(p.extension(), ".tga") == 0)
	{
		FileDisk file;
		if (file.open(pFilename, FileDisk::WRITE))
		{
			TGA_HEADER header;
			memset(&header, 0, sizeof(header));

			header.imagetype = TGA_RGB_UNCOMPRESSED;
			header.bits_per_pixel = 32;
			header.width = ddsd.Width;
			header.height = ddsd.Height;
			header.descriptor = TGA_DESC_FLIPX | TGA_DESC_FLIPY;

			file.write(&header, sizeof(header));

			byte * pPixels = (byte *)lock.pBits;
			for (dword i = 0; i<ddsd.Height; i++)
			{
				file.write(pPixels, ddsd.Width * 4);
				pPixels += lock.Pitch;
			}
		}
	}
	else if (stricmp(p.extension(), ".jpg") == 0
		|| stricmp(p.extension(), ".jpeg") == 0)
	{
		ImageCodec * pCodec = WidgetCast<ImageCodec>(Factory::createNamedWidget("ImageCodecJPEG"));
		if (!pCodec)
			return false;

		// transfer RGBA data from the DX surface into our buffer first..
		Buffer input;
		input.allocate(ddsd.Width * ddsd.Height * 4);

		byte * pInput = (byte *)lock.pBits;
		byte * pOutput = (byte *)input.buffer();

		for (dword i = 0; i<ddsd.Height; ++i)
		{
			for (dword j = 0; j<ddsd.Width; ++j)
			{
				pOutput[0] = pInput[2];
				pOutput[1] = pInput[1];
				pOutput[2] = pInput[0];
				pOutput[3] = 255;

				pOutput += 4;
				pInput += 4;
			}
		}

		Buffer output;
		if (pCodec->encode(input, output, SizeInt(ddsd.Width, ddsd.Height), ImageCodec::CL_NORMAL) > 0)
		{
			FileDisk file;
			if (file.open(pFilename, FileDisk::WRITE))
				file.write(output.buffer(), output.bufferSize());
		}

		delete pCodec;
	}

	primary->UnlockRect();
	RELEASEDD(primary);

	return true;
}

Shader::Ref DisplayDeviceD3D::getShader(const char * pShaderName)
{
	ShaderMap::iterator iShader = m_ShaderMap.find(pShaderName);
	if (iShader != m_ShaderMap.end())
		return iShader->second;

	// insert to the map, if we failed to load this shader, we don't want to keep repeating error messages..
	Shader::Ref pShader = new Shader();
	m_ShaderMap[pShaderName] = pShader;
	pShader->load(this, pShaderName);			// if load fails, it will print any errors to the trace output..

	return pShader;
}

bool DisplayDeviceD3D::releaseShader(const char * pShader)
{
	ShaderMap::iterator iShader = m_ShaderMap.find(pShader);
	if (iShader == m_ShaderMap.end())
		return false;

	iShader->second->release();
	m_ShaderMap.erase(iShader);

	return true;
}

void DisplayDeviceD3D::releaseShaders()
{
	// we have to call release() explicitly, someone else might hang onto a reference to the object preventing it's deletion.
	for (ShaderMap::iterator iShader = m_ShaderMap.begin();
		iShader != m_ShaderMap.end(); ++iShader)
	{
		iShader->second->release();
	}
	m_ShaderMap.clear();
}

void DisplayDeviceD3D::registerEffect(const char * pName, Factory * pFactory)
{
	m_EffectMap[pName] = pFactory;
}

int DisplayDeviceD3D::allocateDVB(int vertexType, int vertexCount,
	LPDIRECT3DVERTEXBUFFER9 & vb)
{
	if (vertexCount > DYNAMIC_VB_SIZE || vertexType < 0 || vertexType >= VB_COUNT)
	{
		TRACE(CharString().format("DisplayDeviceD3D::allocateDVB, vertexType = %d, vertexCount = %d", vertexType, vertexCount));
		return -1;
	}

	// initialize the vertex buffer if needed
	if (m_VB[vertexType] == NULL)
	{
		TRACE(CharString().format("DisplayDeviceD3D::allocateDVB, vertexType = %d", vertexType));

		// create the dynamic VB for this bank and type
		if (m_pD3DD->CreateVertexBuffer(DYNAMIC_VB_SIZE * VERTEX_SIZES[vertexType],
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, VERTEX_FVF[vertexType], D3DPOOL_DEFAULT, &m_VB[vertexType], NULL) != D3D_OK)
		{
			return -1;
		}

		m_VBOffset[vertexType] = 0;
	}

	int space = DYNAMIC_VB_SIZE - m_VBOffset[vertexType];
	if (space < vertexCount)
		m_VBOffset[vertexType] = 0;	// reset the vertex buffer offset if not enough room

									// get the offset
	int offset = m_VBOffset[vertexType];
	// set the vertex buffer pointer
	vb = m_VB[vertexType];
	// update the offset
	m_VBOffset[vertexType] += vertexCount;

	return offset;
}

int DisplayDeviceD3D::setupDVB(int vertexType, int vertexCount, void * pVerts)
{
	ASSERT(vertexType >= 0 && vertexType < VB_COUNT);

	const dword vertSize = VERTEX_SIZES[vertexType];
	const dword vertBytes = vertexCount * vertSize;

	LPDIRECT3DVERTEXBUFFER9 pVB = NULL;
	int	vbOffset = allocateDVB(vertexType, vertexCount, pVB);
	if (vbOffset < 0)
		return -1;
	byte * pBuffer = NULL;
	if (pVB->Lock(vbOffset * vertSize, vertBytes, (void **)&pBuffer, vbOffset > 0 ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD) != D3D_OK)
		return -1;
	if (!pBuffer)
		return -1;

	memcpy(pBuffer, pVerts, vertBytes);

	pVB->Unlock();

	if (m_pD3DD->SetFVF(VERTEX_FVF[vertexType]) != D3D_OK)
		return -1;
	if (m_pD3DD->SetStreamSource(0, pVB, 0, vertSize) != D3D_OK)
		return -1;

	return vbOffset;
}

void DisplayDeviceD3D::resetDVB()
{
	for (int vb = 0; vb<VB_COUNT; vb++)
		m_VBOffset[vb] = 0;
}

void DisplayDeviceD3D::releaseDVB()
{
	for (int vb = 0; vb<VB_COUNT; vb++)
	{
		// release the DX vertex buffer
		RELEASEDD(m_VB[vb]);
		// reset the offset as well
		m_VBOffset[vb] = 0;
	}
}

DWORD DisplayDeviceD3D::getD3DFilterMode() const
{
	static DWORD FILTER_MODES[FILTER_COUNT] =
	{
		D3DTEXF_NONE,			//FILTER_OFF
		D3DTEXF_POINT,			// FILTER_POINT
		D3DTEXF_LINEAR,			// FILTER_LINEAR
		D3DTEXF_ANISOTROPIC,	// FILTER_ANISOTROPIC,
		D3DTEXF_PYRAMIDALQUAD,	// FILTER_PYRAMIDALQUAD
		D3DTEXF_GAUSSIANQUAD,	// FILTER_GAUSSIANQUAD
	};

	return FILTER_MODES[m_eFilterMode];
}

//----------------------------------------------------------------------------

bool DisplayDeviceD3D::initializeDirectX()
{
	D3DPRESENT_PARAMETERS d3dpp;

	// make sure COM is initialized
	if (FAILED(CoInitialize(NULL)))
		return false;

	if (sm_pD3D != NULL)
	{
		m_pD3D = sm_pD3D;
		m_pD3D->AddRef();
	}

	if (m_pD3D == NULL)
	{
		// keep D3D initialized forever, this prevents crashes in windowed mode
		m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
		if (m_pD3D == NULL)
			return false;

		sm_pD3D = m_pD3D;
		sm_pD3D->AddRef();
	}

	if (sm_pD3DD != NULL)
	{
		// DirectX is already initialized, just create an additional swap chain
		m_pD3DD = sm_pD3DD;
		m_pD3DD->AddRef();

		if (!getD3DPP(d3dpp))
			return false;
		if (m_pD3DD->CreateAdditionalSwapChain(&d3dpp, &m_pD3DSC) != D3D_OK)
			return false;

		// get the supported texture formats
		enumerateTextures(d3dpp);
		// get our back and z buffers
		if (!updateBuffers(d3dpp))
			return false;
	}

	if (m_pD3DD == NULL)
	{
		// get the adapter to initialize
		unsigned int adapter = (unsigned int)m_Mode.deviceID;
		// get the presentation parameters
		if (!getD3DPP(d3dpp))
			return false;

#if ENABLE_SHADER_DEBUGGING
		dword flags = D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		if (m_pD3D->CreateDevice(adapter, D3DDEVTYPE_REF, m_HWND, flags, &d3dpp, &m_pD3DD) != D3D_OK)
			return false;
#else
		dword flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
		if (m_pD3D->CreateDevice(adapter, D3DDEVTYPE_HAL, m_HWND, flags, &d3dpp, &m_pD3DD) != D3D_OK)
		{
			flags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			flags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

			if (m_pD3D->CreateDevice(adapter, D3DDEVTYPE_HAL, m_HWND, flags, &d3dpp, &m_pD3DD) != D3D_OK)
				return false;
		}
#endif

		if (m_bWindowed)
		{
			// if windowed, then create another swap chain for this device, this keeps Reset() from being called!
			if (m_pD3DD->CreateAdditionalSwapChain(&d3dpp, &m_pD3DSC) != D3D_OK)
				return false;
		}

		// update the window style flags
		updateWindow();
		// get the supported texture formats
		enumerateTextures(d3dpp);
		// get our back and z buffers
		if (!updateBuffers(d3dpp))
			return false;

		// keep the d3d device for now
		sm_pD3DD = m_pD3DD;
		sm_pD3DD->AddRef();
	}

	// enable lighting!
	m_pD3DD->SetRenderState(D3DRS_LIGHTING, TRUE);
	// turn filtering on
	m_pD3DD->SetSamplerState(0, D3DSAMP_MAGFILTER, getD3DFilterMode());
	m_pD3DD->SetSamplerState(0, D3DSAMP_MINFILTER, getD3DFilterMode());
	m_pD3DD->SetSamplerState(0, D3DSAMP_MIPFILTER, getD3DFilterMode());
	// enable/disable FSAA
	m_pD3DD->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, d3dpp.MultiSampleType != D3DMULTISAMPLE_NONE);
	// set the diffuse source to come from the material
	m_pD3DD->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	m_pD3DD->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	m_pD3DD->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	m_pD3DD->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	m_pD3DD->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	// get the device caps, so we can determine the texture restrictions!
	D3DCAPS9 d3dcaps;
	m_pD3DD->GetDeviceCaps(&d3dcaps);

	m_MaxAnisotropy = d3dcaps.MaxAnisotropy;
	TRACE(CharString().format("MaxAnisotropy : %d", m_MaxAnisotropy));
	m_TextureP2 = (d3dcaps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0;
	m_TextureSquare = (d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0;
	m_TextureMaxSize = SizeInt(d3dcaps.MaxTextureWidth, d3dcaps.MaxTextureHeight);
	m_TextureMinSize = SizeInt(1, 1);		// aparently this is gone in DirectX 8.0

	m_fPixelShaderVersion = D3DSHADER_VERSION_MAJOR(d3dcaps.PixelShaderVersion) +
		(D3DSHADER_VERSION_MINOR(d3dcaps.PixelShaderVersion) / 10.0f);
	TRACE(CharString().format("Pixel Shader : %.1f", m_fPixelShaderVersion));

	m_fVertexShaderVersion = D3DSHADER_VERSION_MAJOR(d3dcaps.VertexShaderVersion) +
		(D3DSHADER_VERSION_MINOR(d3dcaps.VertexShaderVersion) / 10.0f);
	TRACE(CharString().format("Vertex Shader : %.1f", m_fVertexShaderVersion));

	// we require pixel/vertex shader 3.0 for our advanced rendering effects.. if card is less than this, use fixed function pipeline..
	if (m_fVertexShaderVersion < 1.0f || m_fPixelShaderVersion < 1.0f)
		sm_bUseFixedFunction = true;

	// initialize our default shader if we are not using the fixed function pipeline..
	if (m_fVertexShaderVersion >= 1.0f && m_fPixelShaderVersion >= 1.0f
		&& (!m_pDefaultShader.valid() || m_pDefaultShader->released()))
	{
		// load up our default shader..
		m_pDefaultShader = getShader("Shaders/Default.fx");
	}

	// get the current rectangle for the client area
	updateClientArea(false);

	// DirectX initialized successfully..
	return true;
}

bool DisplayDeviceD3D::resetDirectX()
{
	unsigned int adapter = (unsigned int)m_Mode.deviceID;

	// clear our render target so we can reset the device..
	m_pD3DD->SetRenderTarget(0, NULL);
	// release all created effects
	for (WeakEffectList::iterator iEffect = m_CreatedEffects.begin();
		iEffect != m_CreatedEffects.end(); ++iEffect)
	{
		DisplayEffect * pEffect = *iEffect;
		if (pEffect != NULL)
			pEffect->release();
	}

	// release all shadow resources
	m_ShadowPassList.clear();			// the ShadowPass() destructor will release all shadow map textures
	m_bShadowMapReady = false;
	RELEASEDD(m_pShadowMapDS);

	RELEASEDD(m_pBackBuffer);
	RELEASEDD(m_pD3DSC);

	// Release the dynamic vertex buffers and all other DirectX objects that will prevent
	// us from resetting the device.
	releaseDVB();

	// get the presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	if (!getD3DPP(d3dpp))
		return false;

	// reset the device
	if (m_pD3DD->Reset(&d3dpp) != D3D_OK)
	{
		TRACE("DisplayDeviceD3D::resetDirectX() - Reset failed!");
		return false;
	}

	if (m_bWindowed)
	{
		// if entering windowed mode, restore previous window position and size..
		SetWindowPlacement(m_HWND, &m_ClientPlacement);

		if (m_pD3DD->CreateAdditionalSwapChain(&d3dpp, &m_pD3DSC) != D3D_OK)
		{
			TRACE("DisplayDeviceD3D::resetDirectX() - CreateAdditionalSwapChain failed!");
			return false;
		}
	}

	// update the window style flags
	updateWindow();

	if (!updateBuffers(d3dpp))
	{
		TRACE("DisplayDeviceD3D::resetDirectX() - updateBuffers() failed!");
		return false;
	}

	// enable lighting!
	m_pD3DD->SetRenderState(D3DRS_LIGHTING, TRUE);
	// turn filtering on
	m_pD3DD->SetSamplerState(0, D3DSAMP_MAGFILTER, getD3DFilterMode());
	m_pD3DD->SetSamplerState(0, D3DSAMP_MINFILTER, getD3DFilterMode());
	m_pD3DD->SetSamplerState(0, D3DSAMP_MIPFILTER, getD3DFilterMode());
	// enable/disable FSAA
	m_pD3DD->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, d3dpp.MultiSampleType != D3DMULTISAMPLE_NONE);

	return true;
}


void DisplayDeviceD3D::freeDirectX()
{
	releaseDVB();

	// release all created effects
	for (WeakEffectList::iterator iEffect = m_CreatedEffects.begin();
		iEffect != m_CreatedEffects.end(); ++iEffect)
	{
		DisplayEffect * pEffect = *iEffect;
		if (pEffect != NULL)
			pEffect->release();
	}

	// release all shaders
	releaseShaders();

	RELEASEDD(m_pShadowMapDS);

	m_ShadowPassList.clear();			// the ShadowPass() destructor will release all shadow map textures
	m_bShadowMapReady = false;

	RELEASEDD(m_pVS);
	RELEASEDD(m_pVSConstants);
	RELEASEDD(m_pPS);
	RELEASEDD(m_pPSConstants);
	RELEASEDD(m_pBackBuffer);
	RELEASEDD(m_pD3DSC);
	RELEASEDD(m_pD3DD);
	RELEASEDD(m_pD3D);
}

bool DisplayDeviceD3D::readyShadowMap()
{
	if (m_bShadowMapReady)
		return true;		// we are already ready!
	if (!m_bShadowMapSupported)
		return false;
	if (m_bMinimized || m_pD3DD->TestCooperativeLevel() != D3D_OK)
		return false;

	for (ShadowPassList::iterator iPass = m_ShadowPassList.begin();
		iPass != m_ShadowPassList.end(); ++iPass)
	{
		RELEASEDD((*iPass).m_pShadowMap);
	}
	RELEASEDD(m_pShadowMapDS);

	// assume shadow maps are not supported..
	m_bShadowMapSupported = false;
	//m_szShadowMap = m_ClientRectangle.size();

	if (pixelShaderVersion() < 2.0f || vertexShaderVersion() < 1.1f)
		return false;	// pixel shader 2.0 & vertex shader 1.1 are required
	if (!(m_pShadowMap = getShader("Shaders/ShadowMap.fx")))
		return false;
	if (!m_pShadowMap->valid())
		return false;

#if ENABLE_SHADOW_MAP_DS
	if (m_pD3DD->CreateDepthStencilSurface(
		m_szShadowMap.width,
		m_szShadowMap.height,
		findDepthFormat((int)m_Mode.deviceID, D3DFMT_A8R8G8B8),
		D3DMULTISAMPLE_NONE, 0, TRUE, &m_pShadowMapDS, NULL) != D3D_OK)
	{
		TRACE("Failed to create depth surface for shadow maps!");
		return false;
	}
#endif

	m_bShadowMapSupported = true;
	m_bShadowMapReady = true;

	return true;
}

bool DisplayDeviceD3D::validateRenderTarget()
{
	if (!m_bBeginScene)
	{
		// set the render target
		if (m_pD3DD->SetRenderTarget(0, m_pBackBuffer) != D3D_OK)
			return false;
		if (m_pD3DD->BeginScene() != D3D_OK)
			return false;

		m_bBeginScene = true;
	}
	return true;
}

bool DisplayDeviceD3D::getSampleMode(D3DPRESENT_PARAMETERS & d3dpp)
{
	if (m_eFSAA != FSAA_NONE)
	{
		// the order should match the DisplayDevice::FSAA enum
		static D3DMULTISAMPLE_TYPE MODES[] =
		{
			D3DMULTISAMPLE_NONE,
			D3DMULTISAMPLE_NONMASKABLE,
			D3DMULTISAMPLE_2_SAMPLES,
			D3DMULTISAMPLE_3_SAMPLES,
			D3DMULTISAMPLE_4_SAMPLES,
			D3DMULTISAMPLE_5_SAMPLES,
			D3DMULTISAMPLE_6_SAMPLES,
			D3DMULTISAMPLE_7_SAMPLES,
			D3DMULTISAMPLE_8_SAMPLES,
			D3DMULTISAMPLE_9_SAMPLES,
			D3DMULTISAMPLE_10_SAMPLES,
			D3DMULTISAMPLE_11_SAMPLES,
			D3DMULTISAMPLE_12_SAMPLES,
			D3DMULTISAMPLE_13_SAMPLES,
			D3DMULTISAMPLE_14_SAMPLES,
			D3DMULTISAMPLE_15_SAMPLES,
			D3DMULTISAMPLE_16_SAMPLES,
		};

		// validate the anti-aliasing mode, keep reducing until we find a valid mode
		DWORD nQualityLevels = 0;
		for (int i = m_eFSAA; i >= 0; --i)
		{
			D3DMULTISAMPLE_TYPE eSampleType = MODES[i];
			if (eSampleType == D3DMULTISAMPLE_NONE)
				break;

			// check that the card support the FSAA for our HDR buffer type..
			if (m_pD3D->CheckDeviceMultiSampleType((UINT)m_Mode.deviceID,
				m_bHardware ? D3DDEVTYPE_HAL : D3DDEVTYPE_SW, D3DFMT_A16B16G16R16F,
				m_bWindowed, eSampleType, &nQualityLevels) == D3D_OK)
			{
				// validate the depth buffer format is supportted as well...
				if (m_pD3D->CheckDeviceMultiSampleType((UINT)m_Mode.deviceID,
					m_bHardware ? D3DDEVTYPE_HAL : D3DDEVTYPE_SW, d3dpp.AutoDepthStencilFormat,
					m_bWindowed, eSampleType, &nQualityLevels) == D3D_OK)
				{
					d3dpp.MultiSampleType = eSampleType;
					d3dpp.MultiSampleQuality = 0; //nQualityLevels - 1;		// maximum quality
					return true;
				}
			}
		}
	}

	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	return false;
}

bool DisplayDeviceD3D::getD3DPP(D3DPRESENT_PARAMETERS & d3dpp)
{
	memset(&d3dpp, 0, sizeof(d3dpp));

	// get the adapter to initialize
	unsigned int adapter = (unsigned int)m_Mode.deviceID;
	if (m_bWindowed)
	{
		D3DDISPLAYMODE d3ddm;
		if (m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm) != D3D_OK)
			return false;

		RECT clientArea;
		GetClientRect((HWND)m_HWND, &clientArea);

		d3dpp.Windowed = true;
		d3dpp.hDeviceWindow = m_HWND;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferWidth = 0; //(clientArea.right - clientArea.left) + 1;
		d3dpp.BackBufferHeight = 0; // (clientArea.bottom - clientArea.top) + 1;
		d3dpp.BackBufferFormat = d3ddm.Format;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = findDepthFormat((int)m_Mode.deviceID, d3dpp.BackBufferFormat);
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		d3dpp.FullScreen_RefreshRateInHz = 0;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //D3DPRESENT_INTERVAL_DEFAULT;
	}
	else
	{
		d3dpp.hDeviceWindow = m_HWND;
		d3dpp.BackBufferWidth = m_Mode.screenSize.width;
		d3dpp.BackBufferHeight = m_Mode.screenSize.height;
		d3dpp.BackBufferFormat = findFormat(m_Mode.colorFormat);
		d3dpp.BackBufferCount = 1;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; //D3DSWAPEFFECT_FLIP;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = findDepthFormat((int)m_Mode.deviceID, d3dpp.BackBufferFormat);
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		d3dpp.FullScreen_RefreshRateInHz = 0;
		d3dpp.PresentationInterval = sm_bWaitVB ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	getSampleMode(d3dpp);

	return true;
}

bool DisplayDeviceD3D::updateBuffers(D3DPRESENT_PARAMETERS & d3dpp)
{
	RELEASEDD(m_pBackBuffer);

	if (m_pD3DSC != NULL)
	{
		if (m_pD3DSC->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer) != D3D_OK)
			return false;
	}
	else
	{
		ASSERT(m_pD3DD);
		if (m_pD3DD->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer) != D3D_OK)
			return false;
	}

	return true;
}

void DisplayDeviceD3D::updateWindow()
{
	HWND hParent = GetParent(m_HWND);
	if (hParent == NULL || hParent == GetDesktopWindow())
	{
		if (m_bWindowed)
		{
			RECT r;
			::GetWindowRect(m_HWND, &r);

			// make sure caption bar is visible
			int nTop = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
			if (r.top < nTop)
			{
				int nAdjust = nTop - r.top;
				r.bottom += nAdjust;
				r.top += nAdjust;
			}
			int nLeft = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
			if (r.left < nLeft)
			{
				int nAdjust = nLeft - r.left;
				r.right += nAdjust;
				r.left += nAdjust;
			}

			DWORD dwStyle = ::GetWindowLong(m_HWND, GWL_STYLE);
			dwStyle &= WS_POPUP;
			dwStyle |= WS_OVERLAPPEDWINDOW;
			::SetWindowLong(m_HWND, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			DWORD dwExStyle = ::GetWindowLong(m_HWND, GWL_EXSTYLE);
			dwExStyle &= ~WS_EX_TOPMOST;
			::SetWindowLong(m_HWND, GWL_EXSTYLE, dwExStyle);

			::SetWindowPos(m_HWND, HWND_NOTOPMOST,
				r.left, r.top,
				r.right - r.left,
				r.bottom - r.top,
				SWP_FRAMECHANGED | SWP_SHOWWINDOW);

			::ClipCursor(NULL);
		}
		else
		{
			::SetWindowLong(m_HWND, GWL_STYLE, WS_POPUP);

			DWORD dwExStyle = ::GetWindowLong(m_HWND, GWL_EXSTYLE);
			dwExStyle |= WS_EX_TOPMOST;
			::SetWindowLong(m_HWND, GWL_EXSTYLE, dwExStyle);

			::SetWindowPos(m_HWND, HWND_TOPMOST,
				0, 0,
				0,
				0,
				SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

			RECT r;
			::GetWindowRect(m_HWND, &r);
			::ClipCursor(&r);
		}
	}
}

void DisplayDeviceD3D::updateProjection()
{
	// set the clipping window
	D3DVIEWPORT9 d3dvp;
	d3dvp.X = m_Proj.m_rWindow.left;
	d3dvp.Y = m_Proj.m_rWindow.top;
	d3dvp.Width = m_Proj.m_rWindow.width();
	d3dvp.Height = m_Proj.m_rWindow.height();
	d3dvp.MinZ = 0.0f;
	d3dvp.MaxZ = 1.0f;

	m_pD3DD->SetViewport(&d3dvp);

	// set the view matrix
	D3DXMATRIX view;
	setD3DMatrix(view, ~m_Proj.m_mFrame, m_Proj.m_mFrame * -m_Proj.m_vPosition);
	m_pD3DD->SetTransform(D3DTS_VIEW, &view);

	// set the projection matrix
	D3DXMATRIX proj;
	memset(&proj, 0, sizeof(proj));

	if (m_Proj.m_fFOV > 0.0f)	// perspective projection
	{
		float fh = m_Proj.m_fFOV;
		float fv = m_Proj.m_fFOV * (float(m_Proj.m_rWindow.height()) / m_Proj.m_rWindow.width());
		float w = (float)1 / tan(fh * 0.5f);
		float h = (float)1 / tan(fv * 0.5f);
		float Q = m_Proj.m_fBack / (m_Proj.m_fBack - m_Proj.m_fFront);

		proj._11 = w;
		proj._22 = h;
		proj._33 = Q;
		proj._34 = 1.0f;
		proj._43 = -Q * m_Proj.m_fFront;
	}
	else					// orthogonal / parallel projection 
	{
		D3DXMatrixOrthoLH(&proj, m_Proj.m_rWindow.width(),
			m_Proj.m_rWindow.height(),
			m_Proj.m_fFront,
			m_Proj.m_fBack);

		//float s = -m_Proj.m_fFOV;
		//proj._11 = s / m_Proj.m_rWindow.width();
		//proj._22 = s / m_Proj.m_rWindow.height();
		//proj._33 = 1.0f / ( m_Proj.m_fBack - m_Proj.m_fFront );
		//proj._43 = (-m_Proj.m_fFront) / (m_Proj.m_fBack - m_Proj.m_fFront);
		//proj._44 = 1.0f;
	}
	m_pD3DD->SetTransform(D3DTS_PROJECTION, &proj);
}

void DisplayDeviceD3D::enumerateTextures(D3DPRESENT_PARAMETERS & d3dpp)
{
	unsigned int adapter = (unsigned int)m_Mode.deviceID;

	m_TextureFormats.release();

	// add supported texture formats
	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_R8G8B8) == D3D_OK)
		m_TextureFormats.push(ColorFormat::RGB888);
	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8) == D3D_OK)
		m_TextureFormats.push(ColorFormat::RGB8888);
	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8) == D3D_OK)
		m_TextureFormats.push(ColorFormat::RGB888L);
	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_R5G6B5) == D3D_OK)
		m_TextureFormats.push(ColorFormat::RGB565);
	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_X1R5G5B5) == D3D_OK)
		m_TextureFormats.push(ColorFormat::RGB555);
	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A1R5G5B5) == D3D_OK)
		m_TextureFormats.push(ColorFormat::RGB5551);
	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A4R4G4B4) == D3D_OK)
		m_TextureFormats.push(ColorFormat::RGB4444);

	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT1) == D3D_OK)
		m_TextureFormats.push(ColorFormat::DXT1);
	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT3) == D3D_OK)
		m_TextureFormats.push(ColorFormat::DXT3);
	if (m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT5) == D3D_OK)
		m_TextureFormats.push(ColorFormat::DXT5);


	TRACE(CharString().format("DisplayDeviceD3D::enumerateTextures, %d valid texture formats found!", m_TextureFormats.size()));
}

bool DisplayDeviceD3D::updateClientArea(bool a_bAllowReset)
{
	if (m_bWindowed)
	{
		RectInt clientWindow;
		if (IsWindow(m_HWND) && !IsIconic(m_HWND))
		{
			// get the current window placement
			m_ClientPlacement.length = sizeof(m_ClientPlacement);
			GetWindowPlacement(m_HWND, &m_ClientPlacement);

			// get the current window rect
			RECT rect;
			GetClientRect(m_HWND, &rect);
			ClientToScreen(m_HWND, (POINT *)&rect);
			ClientToScreen(m_HWND, ((POINT *)&rect) + 1);

			clientWindow = RectInt(rect.left, rect.top, rect.right - 1, rect.bottom - 1);
			if (!clientWindow.valid())
			{
				clientWindow = RectInt(0, 0, 0, 0);
				m_bMinimized = true;
			}
			else
				m_bMinimized = false;
		}
		else
			m_bMinimized = true;

		if (clientWindow != m_ClientRectangle)
		{
			// window size and position have changed
			m_ClientRectangle = clientWindow;
			// reset directx so we can reallocate the buffers
			if (a_bAllowReset)
				resetDirectX();
		}
	}
	else
	{
		m_ClientRectangle = RectInt(0, 0, m_Mode.screenSize);

		if (IsWindow(m_HWND) && !IsIconic(m_HWND))
			m_bMinimized = false;
		else
			m_bMinimized = true;

	}

	return false;
}

D3DFORMAT DisplayDeviceD3D::findDepthFormat(int adapter, D3DFORMAT adapterFormat)
{
	// default to use a 32-bit depth buffer if available, this should fix alot of the z-fighting..
	if (checkDepthFormat(adapter, adapterFormat, D3DFMT_D32))
		return D3DFMT_D32;
	else if (checkDepthFormat(adapter, adapterFormat, D3DFMT_D24X8))
		return D3DFMT_D24X8;
	else if (checkDepthFormat(adapter, adapterFormat, D3DFMT_D16))
		return D3DFMT_D16;

	// not using stencil's yet, so disabling for now..
	//else if ( checkDepthFormat( adapter, adapterFormat, D3DFMT_D24S8 ) )
	//	return D3DFMT_D24S8;
	//else if ( checkDepthFormat( adapter, adapterFormat, D3DFMT_D24X4S4 ) )
	//	return D3DFMT_D24X4S4;
	//else if ( checkDepthFormat( adapter, adapterFormat, D3DFMT_D15S1 ) )
	//	return D3DFMT_D15S1;

	return D3DFMT_D16;
}

bool DisplayDeviceD3D::checkDepthFormat(int adapter, D3DFORMAT adapterFormat, D3DFORMAT depthFormat)
{
	// Verify that the depth format exists.
	HRESULT hr = m_pD3D->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, adapterFormat,
		D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, depthFormat);

	if (FAILED(hr))
		return false;

	// Verify that the depth format is compatible.
	hr = m_pD3D->CheckDepthStencilMatch(adapter, D3DDEVTYPE_HAL, adapterFormat,
		adapterFormat, depthFormat);

	return SUCCEEDED(hr);
}

bool DisplayDeviceD3D::setPixelShader(IDirect3DPixelShader9 * pPixelShader,
	ID3DXConstantTable * pConstants)
{
	if (sm_bUseFixedFunction)
	{
		clearPixelShader();
		return false;
	}
	if (m_pPS != pPixelShader || m_pPSConstants != pConstants)
	{
		clearPixelShader();
		if (!pPixelShader)
			return true;
		if (m_pD3DD->SetPixelShader(pPixelShader) != D3D_OK)
			return false;

		m_pPS = pPixelShader;
		m_pPS->AddRef();
		m_pPSConstants = pConstants;
		m_pPSConstants->AddRef();
	}

	if (m_pPSConstants != NULL)
	{
		m_pPSConstants->SetDefaults(m_pD3DD);
		updateGlobals(m_pPSConstants);
	}

	return true;
}

bool DisplayDeviceD3D::setVertexShader(IDirect3DVertexShader9 * pVertexShader,
	ID3DXConstantTable * pConstants)
{
	if (sm_bUseFixedFunction)
	{
		clearVertexShader();
		return false;
	}

	if (m_pVS != pVertexShader || m_pVSConstants != pConstants)
	{
		clearVertexShader();

		if (!pVertexShader)
			return true;
		if (m_pD3DD->SetVertexShader(pVertexShader) != D3D_OK)
			return false;

		m_pVS = pVertexShader;
		m_pVS->AddRef();

		m_pVSConstants = pConstants;
		m_pVSConstants->AddRef();
	}

	if (m_pVSConstants != NULL)
	{
		m_pVSConstants->SetDefaults(m_pD3DD);
		updateGlobals(m_pVSConstants);
	}

	return true;
}

void DisplayDeviceD3D::updateGlobals(ID3DXConstantTable * pConstants)
{
	// Set pre-defined constants
	D3DXMATRIX mWorldTransform;
	m_pD3DD->GetTransform(D3DTS_WORLD, &mWorldTransform);
	Shader::applyConstant(this, pConstants, "mWorld", mWorldTransform);

	D3DXMATRIX mViewTransform;
	m_pD3DD->GetTransform(D3DTS_VIEW, &mViewTransform);
	Shader::applyConstant(this, pConstants, "mView", mViewTransform);

	D3DXMATRIX mProjection;
	m_pD3DD->GetTransform(D3DTS_PROJECTION, &mProjection);
	Shader::applyConstant(this, pConstants, "mProj", mProjection);
	Shader::applyConstant(this, pConstants, "vCameraPos", m_Proj.m_vPosition);
}

void DisplayDeviceD3D::clearShaders()
{
	clearPixelShader();
	clearVertexShader();
}

void DisplayDeviceD3D::clearPixelShader()
{
	if (m_pPS != NULL)
	{
		m_pD3DD->SetPixelShader(NULL);

		RELEASEDD(m_pPS);
		RELEASEDD(m_pPSConstants);
	}
}

void DisplayDeviceD3D::clearVertexShader()
{
	if (m_pVS != NULL)
	{
		m_pD3DD->SetVertexShader(NULL);

		RELEASEDD(m_pVS);
		RELEASEDD(m_pVSConstants);
	}
}

//----------------------------------------------------------------------------

void DisplayDeviceD3D::enumerateModes()
{
	static D3DFORMAT SUPPORTED_MODES[] =
	{
		D3DFMT_X8R8G8B8,
		D3DFMT_X1R5G5B5,
	};

	sm_ModeList.release();

	LPDIRECT3D9 D3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (D3D != NULL)
	{
		for (unsigned int i = 0; i<D3D->GetAdapterCount(); i++)
		{
			D3DADAPTER_IDENTIFIER9 d3dai;
			D3D->GetAdapterIdentifier(i, 0, &d3dai);

			for (int k = 0; k<sizeof(SUPPORTED_MODES) / sizeof(D3DFORMAT); ++k)
			{
				for (unsigned int j = 0; j<D3D->GetAdapterModeCount(i, SUPPORTED_MODES[k]); j++)
				{
					D3DDISPLAYMODE d3ddm;
					D3D->EnumAdapterModes(i, SUPPORTED_MODES[k], j, &d3ddm);

					int depth = getFormatDepth(d3ddm.Format);
					if (depth > 0)
					{
						CharString description;
						description.format("%d:%dx%dx%d", i, d3ddm.Width, d3ddm.Height, depth);

						DisplayDevice::Mode newMode;
						newMode.modeDescription = description;
						newMode.deviceID = (void *)i;
						newMode.screenSize = SizeInt(d3ddm.Width, d3ddm.Height);
						newMode.colorFormat = findFormat(d3ddm.Format);

						bool found = false;
						for (int k = 0; k<sm_ModeList.size() && !found; k++)
							if (newMode.modeDescription == sm_ModeList[k].modeDescription)
								found = true;

						if (!found)
							sm_ModeList.push(newMode);
					}
				}
			}
		}

		// release the objects
		RELEASEDD(D3D);
	}
}

ColorFormat::Format DisplayDeviceD3D::findFormat(D3DFORMAT format)
{
	switch (format)
	{
	case D3DFMT_R8G8B8:
		return ColorFormat::RGB888;
	case D3DFMT_A8R8G8B8:
		return ColorFormat::RGB8888;
	case D3DFMT_X8R8G8B8:
		return ColorFormat::RGB888L;
	case D3DFMT_R5G6B5:
		return ColorFormat::RGB565;
	case D3DFMT_X1R5G5B5:
		return ColorFormat::RGB555;
	case D3DFMT_A1R5G5B5:
		return ColorFormat::RGB5551;
	case D3DFMT_A4R4G4B4:
		return ColorFormat::RGB4444;
	case D3DFMT_R3G3B2:
		return ColorFormat::RGB332;
	case D3DFMT_DXT1:
		return ColorFormat::DXT1;
	case D3DFMT_DXT3:
		return ColorFormat::DXT3;
	case D3DFMT_DXT5:
		return ColorFormat::DXT5;
	}

	return ColorFormat::INVALID;
}

D3DFORMAT DisplayDeviceD3D::findFormat(ColorFormat::Format format)
{
	switch (format)
	{
	case ColorFormat::RGB888:
		return D3DFMT_R8G8B8;
	case ColorFormat::RGB8888:
		return D3DFMT_A8R8G8B8;
	case ColorFormat::RGB888L:
		return D3DFMT_X8R8G8B8;
	case ColorFormat::RGB565:
		return D3DFMT_R5G6B5;
	case ColorFormat::RGB555:
		return D3DFMT_X1R5G5B5;
	case ColorFormat::RGB5551:
		return D3DFMT_A1R5G5B5;
	case ColorFormat::RGB4444:
		return D3DFMT_A4R4G4B4;
	case ColorFormat::RGB332:
		return D3DFMT_R3G3B2;
	case ColorFormat::DXT1:
		return D3DFMT_DXT1;
	case ColorFormat::DXT3:
		return D3DFMT_DXT3;
	case ColorFormat::DXT5:
		return D3DFMT_DXT5;
	}

	return D3DFMT_UNKNOWN;
}

int DisplayDeviceD3D::getFormatDepth(D3DFORMAT format)
{
	switch (format)
	{
	case D3DFMT_R8G8B8:
		return 24;
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
		return 32;
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
		return 16;
	case D3DFMT_R3G3B2:
	case D3DFMT_P8:
		return 8;
	}

	return 0;
}

void DisplayDeviceD3D::setD3DMatrix(D3DMATRIX & m, const Matrix33 & mRotation, const Vector3 & vOffset)
{
	m._11 = mRotation.i.x;
	m._12 = mRotation.i.y;
	m._13 = mRotation.i.z;
	m._14 = 0.0f;

	m._21 = mRotation.j.x;
	m._22 = mRotation.j.y;
	m._23 = mRotation.j.z;
	m._24 = 0.0f;

	m._31 = mRotation.k.x;
	m._32 = mRotation.k.y;
	m._33 = mRotation.k.z;
	m._34 = 0.0f;

	m._41 = vOffset.x;
	m._42 = vOffset.y;
	m._43 = vOffset.z;
	m._44 = 1.0f;
}

//------------------------------------------------------------------------------------------------
// EOF
