/*
	NodeSprite.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_SPIRTE_H
#define NODE_SPIRTE_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeSprite : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeSprite>			Ref;
	
	enum AxisLock		// the axis lock is used to determine if the frame of the sprite is
	{					// used for it's transformation or the frame of the camera
		LOCKNONE = 0x0,
		LOCKX = 0x1,	// i.e. locking the Y axis will cause the sprite to keep it's Y axis, while rotating
		LOCKY = 0x2,	// on the X and Z to the camera viewpoint
		LOCKZ = 0x4,
		LOCKXY = LOCKX | LOCKY,
		LOCKXZ = LOCKX | LOCKZ,
		LOCKYZ = LOCKY | LOCKZ,
		LOCKXYZ = LOCKX | LOCKY | LOCKZ,
	};

	// Construction
	NodeSprite();

	// NodeTransform Interface
	BoxHull 			hull() const;
	void				render( RenderContext &context, const Matrix33 & frame, const Vector3 & position );

	// Accessors
	float				scale() const;				// scale of the sprite
	bool				scaleLock() const;			// is scale locked
	bool				clampBack() const;			// keep the sprite from clipping out the back plane
	AxisLock			axisLock() const;			// get the axis lock
	Material::Link		material() const;			// get the assigned material

	// Mutators
	void				setScale( float scale );	
	void				setScaleLock( bool scaleLock );
	void				setClampBack( bool clampBack );
	void				setAlpha( bool enable, float alpha, float alphaV, float alphaA );
	void				setAxisLock( AxisLock lock );
	void				setMaterial( Material::Link material );

private:
	// Data
	float				m_Scale;
	bool				m_ScaleLock;
	bool				m_ClampBack;
	bool				m_EnableAlpha;
	float				m_Alpha, m_AlphaV, m_AlphaA;
	AxisLock			m_AxisLock;
	Material::Link		m_Material;
};

IMPLEMENT_RAW_STREAMING( NodeSprite::AxisLock );

#endif

//----------------------------------------------------------------------------
// EOF
