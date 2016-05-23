/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef $$ROOT$$_H
#define $$ROOT$$_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/BaseNodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef $$ROOT$$_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"$$root$$D.lib") 
#else
#pragma comment(lib,"$$root$$.lib") 
#endif

#endif

//----------------------------------------------------------------------------

class DLL $$root$$ : public BaseNodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<$$root$$>	Ref;

	// Construction
								$$root$$();
	// Widget Interface
	bool						read( InStream & input );
	bool						write( OutStream &output );

	// BaseNodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *			createNode();
	void						initializeNode( BaseNode * pNode );

	// TODO: Add data of this port here
};


#endif

//-------------------------------------------------------------------------------
// EOF
