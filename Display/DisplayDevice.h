/*
	DisplayDevice.h

	Display Device Interface
	(c)2005 Palestar, Richard Lyle
*/

#ifndef DISPLAY_DEVICE_H
#define DISPLAY_DEVICE_H

#include "Factory/FactoryTypes.h"
#include "Standard/Reference.h"
#include "Standard/CharString.h"
#include "Standard/ColorFormat.h"
#include "Standard/Size.h"
#include "Math/Vector3.h"
#include "Math/Matrix33.h"
#include "DevicePrimitive.h"
#include "DisplayEffect.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

const int DEVICE_MODE_WINDOWED = 0;

//----------------------------------------------------------------------------

class PrimitiveSurface;		// forward declare
class PrimitiveMaterial;
class PrimitiveSetTransform;

//----------------------------------------------------------------------------

class DLL DisplayDevice : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference<DisplayDevice>	Ref;

	enum RenderPass
	{
		FIRST_PASS = 0,

		BACKGROUND = 0,			// background polygons
		PRIMARY = 1,			// opaque polygons
		SECONDARY = 2,			// translucent polygons, particle systems, etc

		PASS_COUNT,
	};
	typedef Array< DevicePrimitive::Ref >	PrimitiveArray;

	class Mode
	{
	public:
		Mode() : deviceID( 0 ), screenSize( 1024, 768 ), colorFormat( ColorFormat::RGB888 )
		{}

		CharString				modeDescription;		// Text Description of this mode
		void *					deviceID;				// Device ID
		SizeInt					screenSize;				// Resolution
		ColorFormat::Format		colorFormat;			// ColorFormat

		Mode & operator=( const Mode & copy );
	};
	enum FillMode
	{
		FILL_POINT,
		FILL_WIREFRAME,
		FILL_SOLID,			// default
	};
	enum FogMode
	{
		FOG_NONE,
		FOG_EXP,
		FOG_EXP2,
		FOG_LINEAR,
	};
	enum FilterMode 
	{
		FILTER_OFF,
		FILTER_POINT,
		FILTER_LINEAR,
		FILTER_ANISOTROPIC,
		FILTER_PYRAMIDALQUAD,
		FILTER_GAUSSIANQUAD, 

		FILTER_COUNT
	};
	// full-screen anti-aliasing mode
	enum FSAA
	{
		FSAA_NONE,
		FSAA_NONMASKABLE,
		FSAA_2_SAMPLES,
		FSAA_3_SAMPLES,
		FSAA_4_SAMPLES,
		FSAA_5_SAMPLES,
		FSAA_6_SAMPLES,
		FSAA_7_SAMPLES,
		FSAA_8_SAMPLES,
		FSAA_9_SAMPLES,
		FSAA_10_SAMPLES,
		FSAA_11_SAMPLES,
		FSAA_12_SAMPLES,
		FSAA_13_SAMPLES,
		FSAA_14_SAMPLES,
		FSAA_15_SAMPLES,
		FSAA_16_SAMPLES,

		FSAA_COUNT
	};
	enum LightType {
		LT_INVALID = -1,
		LT_DIRECTIONAL,
		LT_POINT,
		LT_SPOT
	};
	struct Transform {
		Transform() : m_mFrame( Matrix33::IDENTITY ), m_vTranslate( Vector3::ZERO )
		{}

		Matrix33		m_mFrame;
		Vector3			m_vTranslate;
	};


	// Exceptions
	class PrimitiveFailure {};	
	class DeviceNotLocked {};

	// Construction
	DisplayDevice();

	// Accessors
	virtual bool					isLocked() const = 0;				// returns true if the device is currently locked by the calling thread
	virtual int						modeCount() const = 0;				// enumerate available modes
	virtual const Mode *			mode(int n) const = 0;

	virtual ColorFormat *			primaryFormat() const = 0;			// get the color format of the display

	virtual int						surfaceFormatCount() const = 0;		// enumerate available surface color formats
	virtual ColorFormat::Format		surfaceFormat(int n) const = 0;

	virtual const Mode *			activeMode() const = 0;				// current mode
	virtual bool					windowed() const = 0;				
	virtual RectInt					clientWindow() const = 0;			// get the rect of the client window ( in screen coordinates )
	virtual RectInt					renderWindow() const = 0;			// get the rect of the render window ( upper left is always 0,0 )

	virtual bool					textureP2() const = 0;				// does this device require power of two textures
	virtual bool					textureSquare() const = 0;			// does this device require square textures
	virtual SizeInt					textureMaxSize() const = 0;			// maximum texture size
	virtual SizeInt					textureMinSize() const = 0;			// mimimum texture size
	virtual float					pixelShaderVersion() const = 0;		// returns the pixel shader version
	virtual float					vertexShaderVersion() const = 0;	// returns the vertex shader version
	virtual FilterMode				filterMode() const = 0;

	virtual dword					totalVideoMemory() const = 0;		// total video memory available
	virtual dword					freeVideoMemory() const = 0;		// amount of unallocated video memory

	// Mutators
	virtual void					lock() = 0;							// lock this device for multi-threaded access
	virtual void					unlock() = 0;						// unlock this device
	virtual bool					initialize( void *hWnd, const Mode * pMode, 
										bool bWindowed, bool bHardware, FSAA eFSAA = FSAA_NONE ) = 0;
	virtual bool					setMode( const Mode * pMode, bool bWindowed ) = 0;
	virtual void					setFilterMode( FilterMode eMode ) = 0;
	virtual void					release() = 0;						// release this device

	virtual void					flushVideoMemory() const = 0;		// free all video memory being used by textures

	virtual void					clear( Color nColor ) = 0;			// clear the backbuffer to the specific color
	virtual void					clearZ( float fDepth = 1.0f ) = 0;	// clear the z-buffer

	virtual void					setAmbient( Color nColor ) = 0;		// set the ambient light
	virtual int						addDirectionalLight( int nPriority,
										Color nColor,					// color of directional light
										const Vector3 & vDir ) = 0;		// direction of light in view space
	virtual int						addPointLight( int nPriority,		// priority of light, higher priority lights are applied first..
										Color nColor,					// color of point light
										const Vector3 & vPosition,		// position of light in view space
										float fRadius ) = 0;			// radius of light
	virtual void					clearLights() = 0;

	virtual void					setFillMode( FillMode nMode ) = 0;	// set the fill mode
	virtual void					setFog( FogMode nMode,				// set the fog
										float fBegin, float fEnd, Color nColor) = 0;
	virtual void					disableFog() = 0;					// turn off the fog

	virtual void					setProjection( 
										const Matrix33 & mFrame,		// camera orientation in world space
										const Vector3 & vPosition,		// camera position in world space
										const RectInt & rWindow,		// viewport
										float fFOV,						// field of view, negative for orthagonal projection
										float fFront,					// front clipping plane
										float fBack ) = 0;				// back clipping plane
	virtual void					setShadowPass(
										int a_nMaxShadowLights,		// maximum number of lights to project shadows for..
										const Vector3 & a_vFocus,	// focus point in view space for shadows
										float a_fShadowRadius,		// area around the focus point to generate shadow
										const SizeInt & a_szShadowMap ) = 0;	// size of the shadow map in pixels

	// call before pushing any primitives or effects
	virtual bool					beginScene() = 0;	
	// beginShadowPass() should be called until it returns false, each time it returns true
	// the scene should be rendered using the transform passed back in a_LightTransform.
	virtual bool					beginShadowPass( Transform & a_LightTransform ) = 0;
	virtual bool					endShadowPass() = 0;				

	virtual bool					endScene() = 0;						// execute all pushed primitives & effects
	virtual void					abortScene() = 0;					// remove all primitives from stack
	virtual void					present() = 0;						// show rendering results

	virtual DevicePrimitive *		create( const PrimitiveKey &key ) = 0;	
	template<class T> 
	T *								create( Reference<T> &primitive );	
	virtual bool					push( DevicePrimitive * pPrimitive ) = 0;

	virtual DisplayEffect::Ref		createEffect( const char * pName ) = 0;	// create a effect by name
	virtual bool					push( DisplayEffect * pEffect ) = 0;	// starts the provided effect on this display device, effect executes during flush()

	virtual bool					capture( const char * pFilename ) = 0;	// save screenshot to given file, returns true on success!

	// Helpers
	static DisplayDevice *			create( const char * pClass );		// create a DisplayDevice by key
	static DisplayDevice *			create();							// create a default display device

	// Static
	static bool						sm_bUseFixedFunction;				// if true, then the fixed function pipeline will always be used
	static bool						sm_bWaitVB;							// if true, then we will wait for VB
	static dword					sm_nTrianglesRendered;				// total number of triangles rendered
	static dword					sm_nLinesRendered;					// total number of lines rendered
	static CharString				sm_sShadersPath;					// base directory of shader files
	static bool						sm_bEnableShaderDebug;				// if true, then shaders are reloaded from disk when changed
	static DisplayDevice *			sm_pCacheDevice;					// device for used to precache graphics assets...

	static const char *				describeFSAA( FSAA eFSAA );
	static FSAA						findFSAA( const char * pText );
};

//----------------------------------------------------------------------------

inline DisplayDevice::Mode & DisplayDevice::Mode::operator=( const Mode & copy )
{
	modeDescription = copy.modeDescription;
	deviceID = copy.deviceID;
	screenSize = copy.screenSize;
	colorFormat = copy.colorFormat;
	return *this;
}

template<class T> 
inline T * DisplayDevice::create( Reference<T> &primitive )	// throws PrimitiveNotAvailable
{
	return( primitive = (T *)( create( T::staticPrimitiveKey() ) ) );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
