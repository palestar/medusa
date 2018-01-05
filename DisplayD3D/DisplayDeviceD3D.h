/*
	DisplayDeviceD3D.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef DISPLAY_DEVICE_D3D_H
#define DISPLAY_DEVICE_D3D_H

#include "Display/DisplayDevice.h"
#include "Display/PrimitiveMaterial.h"
#include "Display/PrimitiveSetTransform.h"
#include "Standard/Array.h"
#include "Standard/List.h"
#include "Standard/CriticalSection.h"
#include <functional>

#include "Shader.h"

#if _MSC_VER > 1310
	// Fix for VS2005
	#ifndef POINTER_64
	#define POINTER_64		__ptr64
	#endif
#endif

#if defined(WIN32)
#include <D3D9.h>								// DirectX 9.0
#elif defined(XBOX)
#include <xtl.h>
#endif

//---------------------------------------------------------------------------------------------------

//! Define to non-zero to enable shader debugging
#define ENABLE_SHADER_DEBUGGING			0
//! Enable shadown map depth surface
#define ENABLE_SHADOW_MAP_DS			1
//! Define to 1 to enable rendering to shadow map into the back buffer for debugging purposes
#define ENABLE_SHADOW_MAP_DEBUG			0
//! How big to render each shadow map in the main display area
#define DEBUG_SHADOW_MAP_SIZE			256

//----------------------------------------------------------------------------

#define RELEASEDD( qinterface )	if (qinterface != NULL) { qinterface->Release(); qinterface = NULL; }

//----------------------------------------------------------------------------

class PrimitiveSurfaceD3D;		// forward declare

class DisplayDeviceD3D : public DisplayDevice
{
public:
	DECLARE_WIDGET_CLASS();
	
	// Constants
	enum {
		PRIMITIVE_STACK_SIZE	= 1024 * 8,		// the default size for the primitive stacks
		DYNAMIC_VB_SIZE			= 1024 * 32,	// size of the preallocate dynamic vertex buffers

		VB_VERTEX				= 0,			// untransformed and unlit verticies
		VB_VERTEXL				= 1,			// untransformed and lit vertices
		VB_VERTEXTL				= 2,			// transformed and lit verticies
		VB_COUNT				= 3,			// number of streams
	};

	// Types
	typedef Reference<DisplayDeviceD3D>			Ref;
	typedef PrimitiveMaterial::Blending			Blending;
	typedef Array<DisplayDevice::Mode>			ModeList;
	typedef Array<DisplayDeviceD3D *>			DeviceList;

	typedef Array<ColorFormat::Format>						TextureFormat;
	typedef Hash< int, Array< PrimitiveMaterial::Ref > >	StackHash;

	// Construction
	DisplayDeviceD3D();
	virtual ~DisplayDeviceD3D();

	// Accessors
	virtual bool					isLocked() const;				// returns true if the device is currently locked by the calling thread
	virtual int						modeCount() const;				// enumerate available modes
	virtual const Mode *			mode(int n) const;
	virtual ColorFormat *			primaryFormat() const;			// get the color format of the display
	virtual int						surfaceFormatCount() const;		// enumerate available surface color formats
	virtual ColorFormat::Format		surfaceFormat(int n) const;
	virtual const Mode *			activeMode() const;				// current mode
	virtual bool					windowed() const;				
	virtual RectInt					clientWindow() const;			// get the rect of the render surface
	virtual RectInt					renderWindow() const;			// get the render window

	virtual bool					textureP2() const;				// does this device require power of two textures
	virtual bool					textureSquare() const;			// does this device require square textures
	virtual SizeInt					textureMaxSize() const;			// maximum texture size
	virtual SizeInt					textureMinSize() const;			// mimimum texture size
	virtual float					pixelShaderVersion() const;		// returns the pixel shader version
	virtual float					vertexShaderVersion() const;	// returns the vertex shader version
	virtual FilterMode				filterMode() const;				// texture filter mode

	virtual dword					totalVideoMemory() const;		// total video memory available
	virtual dword					freeVideoMemory() const;		// amount of unallocated video memory

	// Mutators
	virtual void					lock();							// lock this device for multi-threaded access
	virtual void					unlock();						// unlock this device
	virtual bool					initialize( void *hWnd, const Mode * pMode, 
										bool bWindowed, bool bHardware, FSAA eFSAA = FSAA_NONE );
	virtual bool					setMode( const Mode * pMode, bool bWindowed );	// change the display mode
	virtual void					setFilterMode( FilterMode eMode );
	virtual void					release();						// release this device

	virtual void					flushVideoMemory() const;		// free all video memory being used by textures

	virtual void					clear( Color nColor );			// clear the backbuffer to the specific color
	virtual void					clearZ( float fDepth = 1.0f );	// clear the z-buffer

	virtual void					setAmbient( Color nColor );		// set the ambient light
	virtual int						addDirectionalLight( int nPriroty, 
										Color nColor,				// color of directional light
										const Vector3 & vDirection );	// direction of light in view space
	virtual int						addPointLight( int nPriority,
										Color nColor,				// color of point light
										const Vector3 & vPosition,	// position of light in view space
										float fRadius );			// radius of light
	virtual void					clearLights();

	virtual void					setFillMode( FillMode nMode );	// set the fill mode
	virtual void					setFog( FogMode nMode, 
										float fBegin, 
										float fEnd, 
										Color nColor);
	virtual void					disableFog();					// turn off the fog

	virtual void					setProjection( 
										const Matrix33 & mFrame,	// camera orientation in world space
										const Vector3 & vPosition,	// camera position in world space
										const RectInt & rWindow,	// viewport
										float fFOV,					// field of view, negative for orthagonal projection
										float fFront,				// front clipping plane
										float fBack );				// back clipping plane
	virtual void					setShadowPass(
										int a_nMaxShadowLights,		// maximum number of lights to project shadows for..
										const Vector3 & a_vFocus,	// focus point in view space for shadows
										float a_fShadowRadius,		// area around the focus point to generate shadow
										const SizeInt & a_szShadowMap );	// size of the shadow map in pixels


	virtual bool					beginScene();
	virtual bool					beginShadowPass( Transform & a_LightTransform );
	virtual bool					endShadowPass();
	virtual bool					endScene();						// execute all pushed primitives
	virtual void					abortScene();					// remove all primitives from stack
	virtual void					present();						// show rendering results

	virtual DevicePrimitive *		create( const PrimitiveKey &key );	
	virtual bool					push( DevicePrimitive * primitive );

	virtual DisplayEffect::Ref		createEffect( const char * pName );	// create a effect by name, returns NULL if effect is not found
	virtual bool					push( DisplayEffect * pEffect );	// starts the provided effect on this display device

	virtual bool					capture( const char * pFilename );	// save primary display to file

	// pixel/vertex shader management
	Shader::Ref						getShader( const char * pShader );		// returns a shader by name, will load and compile if required.
	bool							releaseShader( const char * pShader );	// release a specific shader
	void							releaseShaders();						// releases all shaders, used when DirectX needs to reset

	// register a effect factory by name
	void							registerEffect( const char * pName, 
										Factory * pFactory );

	// allocate dynamic vertex buffer space
	int								allocateDVB( int vertexType, int vertexCount,
										LPDIRECT3DVERTEXBUFFER9 & vb );								// reutrns the vertex buffer offset
	int								setupDVB( int vertexType, int vertexCount, void * pVerts );		// returns the offset into the current vertex buffer
	void							resetDVB();
	void							releaseDVB();

	DWORD							getD3DFilterMode() const;		// get the D3D filter mode value

	//----------------------------------------------------------------------------

	//! Types
	typedef std::multimap< int, D3DLIGHT9, std::greater<int> >		LightMap;
	typedef std::map< CharString, Shader::Ref >						ShaderMap;
	typedef std::map< CharString, Factory * >						EffectMap;
	typedef std::list< DisplayEffect::Ref >							EffectList;
	typedef std::list< DisplayEffect::WeakRef >						WeakEffectList;

	struct Projection {
		Projection() : m_mFrame( Matrix33::IDENTITY ),
			m_vPosition( Vector3::ZERO ),
			m_rWindow( PointInt(0,0), SizeInt(1024,1024) ),
			m_fFOV( PI / 4.0f ),
			m_fFront( 1.0f ),
			m_fBack( 20000.0f )
		{};

		Matrix33						m_mFrame;			// rotation of the camera in world space
		Vector3							m_vPosition;		// position of the camera in world space
		RectInt							m_rWindow;			// viewport
		float							m_fFOV;				// field of view
		float							m_fFront;			// front clipping plane
		float							m_fBack;			// back clipping plane
	};

	struct ShadowPass 
	{
		ShadowPass() : m_pShadowMap( NULL ), m_bOrthoProj( false )
		{}
		~ShadowPass()
		{
			RELEASEDD( m_pShadowMap );
		}

		Array< DevicePrimitive::Ref >	m_Primitives;
		Transform						m_LightTransform;			// transform for the current pass
		D3DXMATRIX						m_LightView;				// light view-space transform
		D3DXMATRIX						m_LightProj;				// light projection transform
		IDirect3DTexture9 *				m_pShadowMap;
		bool							m_bOrthoProj;
	};
	typedef std::list< ShadowPass >		ShadowPassList;

	// Data
	CriticalSection					m_Lock;							// semaphore for locking this device
	dword							m_dwLockingThread;				// ID of the thread that has this device locked..
	dword							m_nLockCount;					// how many times has it been locked by the same thread

	bool							m_bBeginScene;					// set to true once beginScene() has been called
	Array< PrimitiveMaterial::Ref >	m_Stack[ PASS_COUNT ];			// primitive stacks
	PrimitiveMaterial::Ref			m_pCurrentMaterial;				// last pushed/executed material
	int								m_nTextureStage;				// current texture stage, incremented by each PrimitiveSurfaceD3D executed
	PrimitiveSetTransform::Ref		m_pCurrentTransform;			// last pushed/executed transform

	TextureFormat					m_TextureFormats;				// collection of supported texture formats
	bool							m_TextureP2;					// textures must be power of two
	bool							m_TextureSquare;				// textures must be square
	SizeInt							m_TextureMaxSize;				// maximum size of an texture
	SizeInt							m_TextureMinSize;
	float							m_fPixelShaderVersion;
	float							m_fVertexShaderVersion;
	FilterMode						m_eFilterMode;					// texture filter mode
	int								m_MaxAnisotropy;				// max anisotropic filtering level

	HWND							m_HWND;							// window handle device is attached too
	bool							m_bHardware;					// hardware or software rendering
	ColorFormat::Ref				m_pFormat;						// format of the primary surface
	Mode							m_Mode;							// mode of this device
	bool							m_bWindowed;					// true if in windowed mode
	FSAA							m_eFSAA;						// full-screen anti aliasing mode

	RectInt							m_ClientRectangle;				// Window client area in screen space
	bool							m_bMinimized;					// has window been minimized?
	WINDOWPLACEMENT					m_ClientPlacement;				// Window placement
	Projection						m_Proj;							// current projection data

	LPDIRECT3D9						m_pD3D;							// Direct3D object
	LPDIRECT3DDEVICE9 				m_pD3DD;						// Direct3D Device object
	LPDIRECT3DSWAPCHAIN9			m_pD3DSC;						// Direct3D swap chain

	LPDIRECT3DSURFACE9				m_pBackBuffer;					// our back buffer

	Color							m_cAmbientLight;				// amount of ambient light
	LightMap						m_Lights;						// our lights
	dword							m_NextLightId;					// the next light id
	
	dword							m_VBOffset[VB_COUNT];			// next offsets into the vertex buffer
	LPDIRECT3DVERTEXBUFFER9			m_VB[VB_COUNT];					// dynamic vertex buffers

	ShaderMap						m_ShaderMap;					// shader name -> Shader
	IDirect3DVertexShader9 *		m_pVS;							// current vertex shader
	ID3DXConstantTable *			m_pVSConstants;					// current vertex shader constants
	IDirect3DPixelShader9 *			m_pPS;							// current pixel shader
	ID3DXConstantTable *			m_pPSConstants;					// current pixel shader constants

	EffectMap						m_EffectMap;					// effect name -> Effect Factory
	WeakEffectList					m_CreatedEffects;				// list of all effects created.. even those not attached
	EffectList						m_EffectList;					// list of currently attached effects

	bool							m_bShadowMapReady;				// true if after first time initialization has been
	bool							m_bShadowMapSupported;			// true if shadow maps are supported on this hardware
	Vector3							m_vShadowFocus;					// focus point for shadows in view space
	float							m_fShadowRadius;				// how big of an area to shadow..
	SizeInt							m_szShadowMap;					// size of our shadow map
	bool							m_bShadowPass;					// true if beginShadowPass() has been called
	int								m_nMaxShadowLights;				// maximum number of lights to perform shadow maps
	int								m_nShadowMapPass;				// current shadow map pass
	ShadowPassList					m_ShadowPassList;				// shadow maps for each light
	IDirect3DSurface9 *				m_pShadowMapDS;					// depth surface (optional)

	bool							m_bFirstShadowPass;
	LightMap::iterator				m_iCurrentShadowLight;
	ShadowPassList::iterator		m_iCurrentShadowPass;
	bool							m_bUsingFixedFunction;			// true if we are currently using the fixed function pipeline, set by PrimitiveMaterialD3D..

	Shader::Ref						m_pDefaultShader;				// default vertex/pixel shader
	Shader::Ref						m_pShadowMap;					// vertex/pixel shader used to generate the shadow map
	Shader::Ref						m_pMatShader;					// current shader from the PrimitiveMaterial, needed by PrimitiveSurface..

	// Static
	static LPDIRECT3D9				sm_pD3D;						// Direct3D object
	static LPDIRECT3DDEVICE9 		sm_pD3DD;						// Direct3D Device object
	static ModeList					sm_ModeList;					// list of display modes
	static DeviceList				sm_DeviceList;					// list of available render devices

	// Mutators
	bool							updateClientArea( bool a_bAllowReset );

	bool							initializeDirectX();
	bool							resetDirectX();
	void							freeDirectX();
	bool							readyShadowMap();

	bool							validateRenderTarget();
	bool							getSampleMode( D3DPRESENT_PARAMETERS & d3dpp );
	bool							getD3DPP( D3DPRESENT_PARAMETERS & d3dpp );
	bool							updateBuffers( D3DPRESENT_PARAMETERS & d3dpp );
	void							updateWindow();
	void							updateProjection();
	void							enumerateTextures( D3DPRESENT_PARAMETERS & d3dpp );

	D3DFORMAT						findDepthFormat( int adapter, D3DFORMAT adapterFormat );
	bool							checkDepthFormat( int adapter, D3DFORMAT adapterFormat, D3DFORMAT depthFormat );

	bool							setPixelShader( IDirect3DPixelShader9 * pPixelShader,
										ID3DXConstantTable * pConstants );
	bool							setVertexShader( IDirect3DVertexShader9 * pVertexShader,
										ID3DXConstantTable * pConstants );
	void							updateGlobals( ID3DXConstantTable * pConstants );
	void							clearShaders();
	void							clearPixelShader();
	void							clearVertexShader();

	// Static
	static void						enumerateModes();
	static ColorFormat::Format		findFormat( D3DFORMAT format );
	static D3DFORMAT				findFormat( ColorFormat::Format format );
	static int						getFormatDepth( D3DFORMAT format );

	static void						setD3DMatrix( D3DMATRIX & m, const Matrix33 & mRotation, const Vector3 & vOffset );
};

//----------------------------------------------------------------------------

#endif

//------------------------------------------------------------------------------------
// EOF
