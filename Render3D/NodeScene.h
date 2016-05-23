/*
	NodeScene.h

	This node is used to link one scene to another scene
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_SCENE_H
#define NODE_SCENE_H

#include "Render3D/Scene.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeScene : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeScene>			Ref;

	// Construction
	NodeScene();
	NodeScene( Scene * pScene, bool bNormalize = true );
	virtual ~NodeScene();

	// NodeTransform Interface
	BoxHull				hull() const;

	void				render( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );

	// Accessors
	Scene *				scene() const;
	// Mutators
	void				setScene( Scene * pScene, bool bNormalize = true );

private:
	Scene::Link			m_pScene;
	bool				m_bNormalizeFrame;	
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
