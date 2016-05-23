/*
	RenderContext.h

	The RenderContext object is passed to all nodes being rendered in a heirarchy. It contains render state information
	for rendering nodes into a display device (i.e. window, position, frame, etc..)

	This object also does sphere/box visibility testing and translucensy sorting in addition it handles tracking instance
	data and performing shadow map generation.

	Rendering using the RenderContext should follow the flow detailed below:

	if ( context.beginScene() )
	{
		// render scene including lights

		// enumerate each light, render the scene from the perspective of the light..
		while( context.beginShadowPass() )
		{
			// render scene again, any lights will be ignored in this pass..

			context.endShadowPass();
		}

		context.endScene();
	}


	NOTE: If you use setWindow() or setProjection(), any changes will not take effect until you call endScene().

	(c)2005 Palestar, Richard Lyle
*/

#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include "Math/Matrix33.h"
#include "Math/Vector3.h"
#include "Math/Plane.h" 
#include "Display/DisplayDevice.h"
#include "Audio/AudioDevice.h"
#include "Math/BoxHull.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class Scene;	// forward declare
class BaseNode;
class NodeLight;
class Material;

class DLL RenderContext
{
public:
	static float			sm_fDefaultDetail;			// default detail level, should be initialized by client options..
	static int				sm_nShadowMapSize;			// default shadow level, should be initialized by client options..
	static float			sm_fShadowMapRadius;		// default shadow map radius

	//! This class is the base class for any type instance data needed by the nodes being rendered.
	class DLL InstanceData : public Widget
	{
	public:
		DECLARE_WIDGET_CLASS();
		
		//! Types
		typedef Reference< InstanceData >		Ref;
		//! Constructor
		InstanceData();
		//! Accessors
		bool			isTouched() const;			// true if the instance data has been touched, set to false at beginning of rendering.
		//! Mutators
		void			setTouched( bool a_bTouched );
	protected:
		//! Data
		bool			m_bTouched;
	};

	//! Handy scoped based object for pushing then poping the state on destruction.
	class SaveState 
	{
	public:
		SaveState( RenderContext & context ) : m_Context( context )
		{
			m_Context.pushState();
		}
		~SaveState()
		{
			m_Context.popState();
		}
	private:
		RenderContext &	m_Context;
	};

	//! Construction
	RenderContext();
	RenderContext( DisplayDevice * pDisplay, AudioDevice * pAudio,
		float time, dword bits, const Matrix33 &frame, const Vector3 &position,
		const RectInt & window, float fov, float front, float back );

	// Accessors
	DisplayDevice *		display() const;									// current render device
	AudioDevice *		audio() const;										// current audio device
	float				time() const;										// render @ time
	dword				bits() const;										// render bits
	qword				instanceKey() const;								// render instance key
	float				alpha() const;										// alpha scalar, 0.0 - 1.0
	float				detail() const;										// detail level, 0.0 (lowest detail) - 1.0 (highest detail)
	bool				isShadowPass() const;								// true if this is a shadow pass render
	bool				isORenabled() const;								// true if Oculus Rift mode is enabled

	const Matrix33 &	frame() const;										// world space frame and position
	const Vector3 &		position() const;

	const RectInt &		window() const;										// projection information
	const PointInt &	offset() const;
	float				fov() const;
	float				front() const;
	float				back() const;

	float				fps() const;										// current frames per second
	float				elapsed() const;									// based on the fps, what amount of time has elapsed since the last render

	
	bool				boxVisible( const BoxHull & hull ) const;			// hull should already be in view space
	bool				boxVisible( const BoxHull & hull,					// min/max in object space
								const Matrix33 & rotate, 
								const Vector3 & translate ) const;

	bool				sphereVisible( const Vector3 &center,				// center in view space
								float radius ) const;
	bool				clip( Vector3 & clip );								// clip vector, should be in viewspace

	void				project( Vector3 & threeD );						// project viewspace to screenspace
	void				unProject( Vector3 &twoD );							// unproject from screenspace to viewspace
	void				unProjectFloat( Vector3 & twoD );					// unproject from screenspace ( 0.0 - 1.0 ) to viewspace

	Vector3				worldToView( const Vector3 & position );			// translate from world to view space
	Matrix33			worldToView( const Matrix33 & frame );
	BoxHull				worldToView( const BoxHull & hull );
	Vector3				viewToWorld( const Vector3 & position );			// translate from view space to world
	Matrix33			viewToWorld( const Matrix33 & frame );					

	// Mutators
	void				setDisplay( DisplayDevice * pDisplay );
	void				setAudio( AudioDevice * pAudio );
	void				setTime( float time );
	void				setBits( dword bits );
	void				setInstanceKey( qword instanceKey );
	InstanceData *		instanceData( const ClassKey & a_ClassKey );		// get the current instance data, creates the data if needed.
	void				setAlpha( float fAlpha );
	void				setDetail( float fDetail );

	void				setFrame( const Matrix33 & frame );					// set camera frame/position in world space
	void				setPosition( const Vector3 & position );
	void				setProjection( const RectInt & window, 
							float fov, float front, float back );			// set the projection
	void				setMaxShadowLights( int a_nLights );
	void				setShadowFocus( const Vector3 & a_vFocus,			// set our focus area for shadows
							float a_fRadius );
	void				setShadowMap( const SizeInt & a_szShadowMap );
	void				setWindow( const RectInt & window );				// set the render window
	void				setOffset( const PointInt & offset );				// set a window offset
	void				updateTransforms();									// update current view & projection transforms

	void				enableOR( float a_fMetersPerUnit = 1.0f );			// enable Oculus Rift mode, the optional parameter is used to convert from 
																			// in-game space to actual distance in meters. 
	void				disableOR();										// turn off Oculus Rift mode

	void				lock();												// lock the display device
	void				unlock();

	void				beginRender( Color clearColor, Color ambient );		// clear the display and z buffer
	void				render( Scene * pScene );							// render the scene
	void				render( BaseNode * pNode );							// render the node

	void				push( DevicePrimitive * pPrimitive );				// push a display primitive
	void				push( Material * pMaterial );						// push the provided material object

	//! This function pushes a transform to transform local coordinates into view space
	//! using a view-space frame & position.
	void				pushTransform( const Matrix33 & frame,				
							const Vector3 & position );					
	//! This function pushes a transform for local coordinates into view space
	//! using a world-space frame & position.
	void				pushWorldTransform( const Matrix33 & frame,
							const Vector3 & position );					
	//! If your coordinates are already in view-space, you can pushIndentity() to 
	//! clear any previously pushed transform.
	void				pushIdentity();

	// beginScene() is called to begin rendering the scene.
	bool				beginScene();			
	// beginShadowPass() should be called after beginScene(), the entire scene should be pushed again
	// before calling endShadowPass(), if no additional maps then beginShadowPass() this will return false.
	bool				beginShadowPass();
	void				endShadowPass();
	// endScene() should be called after all passes have completed.
	bool				endScene();
	// abortScene() should be called to abort all pushed draw commands 
	bool				abortScene();

	void				endRender();										// flip the display and update the frames per second
	void				flushInstanceData();								// flush all instance data from the context

	bool				pushState();										// push the current state into an internal stack
	bool				popState();											// return the state to a previously pushed state

	// Helpers
	void				renderBox( BoxHull &hull, Color color );
	void				renderPlane( const Plane & plane, Color color );
	void				renderCircle( float radius, Color color, int segments );

	// obsolete
	DisplayDevice *		device() const;										// current render device
	AudioDevice *		audioDevice() const;								// current audio device


	//! Template function to get the instance data of a given type, type must use DECLARE_WIDGET_CLASS().
	template<typename T>
	T * instanceData()
	{
		return WidgetCast<T>( instanceData( T::classKey() ) );
	}

private:
	// Disallow copy of RenderContext
	RenderContext( const RenderContext & a_Copy )
	{}

	// Types
	enum FulstrumPlanes
	{
		FRONT,
		BACK,
		RIGHT,
		LEFT,
		TOP,
		BOTTOM,

		FULSTRUM_COUNT
	};
	typedef std::map< qword, InstanceData::Ref >		InstanceDataMap;
	
	struct State 
	{
		State() :
			m_Time( 0.0f ),
			m_Bits( 0 ),
			m_nInstanceKey( 0 ),
			m_fAlpha( 1.0f ),
			m_fDetail( sm_fDefaultDetail ),
			m_Frame( Matrix33::IDENTITY ),
			m_Position( Vector3::ZERO ),
			m_bShadowPass( false ),
			m_bOR( false ),
			m_nMaxShadowLights( 0 ),
			m_vShadowFocus( 0, 0, 1 ),
			m_fShadowRadius( sm_fShadowMapRadius ),
			m_szShadowMap( sm_nShadowMapSize, sm_nShadowMapSize ),
			m_Offset( 0, 0 ),
			m_Fov( PI / 4 ),
			m_Front( 1.0f ),
			m_Back( 32768.0f ),
			m_Clock( 0 ),
			m_Fps( 30.0f ),
			m_Elapsed( 1.0f / 30.0f )
		{
			m_Window.setInvalid();
		}

		float				m_Time;
		dword				m_Bits;
		qword				m_nInstanceKey;
		float				m_fAlpha;
		float				m_fDetail;

		RectInt				m_Window;							// projection data
		PointInt			m_Offset;							// 2D offset
		float				m_Fov;
		float				m_Front;
		float				m_Back;
		PointFloat			m_Center;
		float				m_D;
		float				m_Q;
		float				m_W;
		Plane				m_Fulstrum[ FULSTRUM_COUNT ];		// planes for clip testing

		Vector3				m_Position;							// position and frame of camera
		Matrix33			m_Frame;
		bool				m_bShadowPass;						// true if this is a shadow pass
		bool				m_bOR;								// true if Oculus rift is enabled
		int					m_nMaxShadowLights;
		Vector3				m_vShadowFocus;
		float				m_fShadowRadius;
		SizeInt				m_szShadowMap;

		dword				m_Clock;							// used to calculate the frames per second
		float				m_Fps;	
		float				m_Elapsed;
	};
	typedef std::list< State >				StateList;

	// Data
	DisplayDevice::Ref	m_Display;							// devices
	AudioDevice::Ref	m_Audio;
	State				m_State;
	StateList			m_StateList;

	InstanceDataMap		m_InstanceDataMap;
};

//-------------------------------------------------------------------------------

inline DisplayDevice * RenderContext::display() const
{
	return m_Display;
}

inline AudioDevice * RenderContext::audio() const
{
	return m_Audio;
}

inline float RenderContext::time() const
{
	return m_State.m_Time;
}

inline dword RenderContext::bits() const
{
	return m_State.m_Bits;
}

inline qword RenderContext::instanceKey() const
{
	return m_State.m_nInstanceKey;
}

inline float RenderContext::alpha() const
{
	return m_State.m_fAlpha;
}

inline float RenderContext::detail() const
{
	return m_State.m_fDetail;
}

inline bool RenderContext::isShadowPass() const
{
	return m_State.m_bShadowPass;
}

inline bool RenderContext::isORenabled() const
{
	return m_State.m_bOR;
}

inline const Matrix33 & RenderContext::frame() const
{
	return m_State.m_Frame;
}

inline const Vector3 & RenderContext::position() const
{
	return m_State.m_Position;
}

inline const RectInt & RenderContext::window() const
{
	return m_State.m_Window;
}

inline const PointInt &	RenderContext::offset() const
{
	return m_State.m_Offset;
}

inline float RenderContext::fov() const
{
	return m_State.m_Fov;
}

inline float RenderContext::front() const
{
	return m_State.m_Front;
}

inline float RenderContext::back() const
{
	return m_State.m_Back;
}

inline float RenderContext::fps() const
{
	return m_State.m_Fps;
}

inline float RenderContext::elapsed() const
{
	return m_State.m_Elapsed;
}

//---------------------------------------------------------------------------------------------------

inline RenderContext::InstanceData::InstanceData() : m_bTouched( false )
{}

inline bool RenderContext::InstanceData::isTouched() const
{
	return m_bTouched;
}

inline void RenderContext::InstanceData::setTouched( bool a_bTouched )
{
	m_bTouched = a_bTouched;
}

//----------------------------------------------------------------------------

// obsolete
inline DisplayDevice * RenderContext::device() const
{
	return( m_Display );
}

inline AudioDevice * RenderContext::audioDevice() const
{
	return( m_Audio );
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
