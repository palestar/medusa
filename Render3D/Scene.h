/*
	Scene.h

	This is used as a container object for a 3D scene and it's animations
	(c)2005 Palestar, Richard Lyle
*/

#ifndef	SCENE_H
#define SCENE_H

#include "Resource/Resource.h"
#include "NodeTransform.h"
#include "Render3dDll.h"

//----------------------------------------------------------------------------

class DLL Scene : public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef ResourceLink<Scene>		Link;
	typedef Reference<Scene>		Ref;

	struct IntersectContext
	{
		Vector3			origin;					// ray origin
		Vector3			ray;					// ray direction

		Vector3			intersect;				// intersect point
		BaseNode *		intersectNode;
		float			intersectDistance2;		// distance from origin, magnitude2()
	};
	struct Segment
	{
		dword			hashId;
		float			beginTime;
		float			endTime;
	};

	// Construction
	Scene();
	Scene( BaseNode * pNode );

	// Accessors
	BaseNode *			node() const;						// root node of the scene
	float				fps() const;						// render animation rate

	int					findSegment( const char * string ) const;
	int					segmentCount() const;
	const Segment &		segment( int n ) const;

	Matrix33			defaultCameraFrame() const;
	Vector3				defaultCameraPosition() const;

	BoxHull				calculateHull() const;						// get the entire hull of the scene

	// Mutators
	void				setNode( BaseNode * pNode );
	void				setFps( float fps );
	int					addSegment( const Segment &segment );
	void				removeSegment( int n );

	void				setDefaultCamera( const Matrix33 & frame, const Vector3 & position );

	// test for intersection of a ray with the nodes in the scene
	bool				intersect( const Matrix33 &frame, const Vector3 &position,
							IntersectContext & context );		
	
	// render nodes in scene
	//
	void				render( RenderContext &context,	const Matrix33 & frame, const Vector3 & position );

	// Static
	static Scene *		currentScene();

private:
	// helpers
	void				enumBoundingBox( const Matrix33 &rotation, const Vector3 &translation, 
							BaseNode *pBaseNode, BoxHull &hull ) const;
	void				enumIntersect( BaseNode *pBaseNode, const Matrix33 &rotation, const Vector3 &translation,
							IntersectContext & context ) const;

	// Data
	BaseNode::Ref		m_Node;		
	float				m_Fps;

	Array< Segment >	m_Segments;								// animation segments

	Matrix33			m_DefaultCameraFrame;
	Vector3				m_DefaultCameraPosition;

	static Scene *		s_pCurrentScene;
};

IMPLEMENT_RAW_STREAMING( Scene::Segment );

//-------------------------------------------------------------------------------

inline Scene * Scene::currentScene()
{
	return( s_pCurrentScene );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
