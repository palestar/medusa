/*
	NodeTransform.h

	BaseNode with position & frame
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_TRANSFORM_H
#define NODE_TRANSFORM_H

#include "BaseNode.h"
#include "Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeTransform : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeTransform>		Ref;

	// Construction
	NodeTransform();
	NodeTransform( const Matrix33 &frame, const Vector3 &position );
	NodeTransform( NodeTransform * pParent, const Matrix33 &frame, const Vector3 &position );
	NodeTransform( const NodeTransform & copy );

	// Accessors
	const Vector3 &			position() const;
	const Matrix33 &		frame() const;

	// Mutators
	virtual void			setPosition( const Vector3 &position );
	virtual void			setFrame( const Matrix33 &frame );
	
	// BaseNode Interface
	virtual void			render( RenderContext &context, 
								const Matrix33 & frame, const Vector3 & position );
	virtual void			preRender( RenderContext &context, 
								const Matrix33 & frame, const Vector3 & position );

	// Static
	static Vector3			worldPosition( BaseNode * pNode );
	static Matrix33			worldFrame( BaseNode * pNode );
	static BoxHull			worldHull( BaseNode * pNode );

	static bool				s_bDrawNodeAxis;					// draw node axis
	static bool				s_ShowHull;							// show rotated hull around nodes
	static bool				s_ShowAlignedHull;					// show axis-aligned hull around nodes

protected:
	// Data
	Vector3					m_Position;							// position and frame
	Matrix33				m_Frame;		
};

//----------------------------------------------------------------------------

inline const Vector3 & NodeTransform::position() const
{
	return( m_Position );
}

inline const Matrix33 &	NodeTransform::frame() const
{
	return( m_Frame );
}

//-------------------------------------------------------------------------------

inline void NodeTransform::setPosition( const Vector3 &position )
{
	m_Position = position;
}

inline void NodeTransform::setFrame( const Matrix33 &frame )
{
	m_Frame = frame;
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
