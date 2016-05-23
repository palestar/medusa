/*
	BaseNodePort.h

	Class for adding new types of nodes to the ScenePort
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BASE_NODE_PORT_H 
#define BASE_NODE_PORT_H

#include "Standard/CharString.h"
#include "Standard/Referenced.h"
#include "Factory/Widget.h"
#include "Render3d/BaseNode.h"
#include "Render3d/Material.h"

#include "Tools/ResourcerDoc/Port.h"
#include "Tools/ScenePort/ScenePortDll.h"

//-------------------------------------------------------------------------------

#define MAX_NODE_PORTS						(0x100)
#define ID_NODE_PORT_BEGIN					(64000)
#define ID_NODE_PORT_END					(ID_NODE_PORT_BEGIN + MAX_NODE_PORTS)

#define NODE_PROPERTIES_WIDTH				(250)

//----------------------------------------------------------------------------

class ScenePort;	// forward declare

class DLL BaseNodePort : public CDocument, public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<BaseNodePort>			Ref;
	typedef Array< CharString >				DependentArray;
	typedef Array< CharString * >			StringArray;

	typedef Resource						Resource;
	typedef InStream						InStream;
	typedef OutStream						OutStream;
	typedef ClassKey						ClassKey;
	typedef WidgetKey						WidgetKey;
	typedef Factory							Factory;
	typedef File							File;
	typedef CharString						CharString;
	typedef String							String;

	typedef Vector3							Vector3;
	typedef Matrix33						Matrix33;

	typedef BaseNode						BaseNode;

	// Construction
	BaseNodePort();
	virtual ~BaseNodePort();

	// PropertyClass interface
	virtual bool				copy( void * pLeft, Type * pLeftType, const void * pRight, Type * pRightType, 
									Property * pProperty, int nElement, bool bMutator );
	// Widget Interface
	bool						read( const InStream & input );
	// Accessors
	bool						update() const;
	const char *				name() const;
	int							childCount() const;
	BaseNodePort *				child(int n) const;

	CDocument *					parent() const;

	// Mutators
	void						setUpdate( bool update = true, bool recursive = false );
	void						setName( const char * name );
	void						addChild( BaseNodePort * node );
	void						removeChild( int n );

	void						setParent( CDocument * pParent );
	void						newKey( bool recursive = true );

	// Helpers
	bool						isChild( BaseNodePort *node ) const;
	void						updateParent() const;
	void						updateAllViews();					// update views of the node itself and the parent document views!

	BaseNodePort *				findNode( const char * name );
	BaseNodePort *				findNode( const WidgetKey & key );

	BaseNode *					findCachedNode( const char * pName );
	BaseNode *					findCachedNode( const WidgetKey & nKey );
	BaseNode *					updateNodeCache( bool bForceUpdate = false, bool bCheckKeys = false );
	BaseNode *					getCachedNode();
	
	void						recurseDependencies( DependentArray & dep );

	void						report( const char * pMessage );

	//----------------------------------------------------------------------------

	virtual bool				import( const char * pFileName );

	virtual void				dependencies( DependentArray & dep );
	virtual CFrameWnd *			createView();
	
	virtual BaseNode *			createNode();
	virtual void				initializeNode( BaseNode * pNode );
	
	// Interface for ScenePort
	//
	virtual bool				canMove();
	virtual void				moveXY( int x, int y );
	virtual void				move( Vector3 & move );
	virtual bool				canScale();
	virtual void				scale( int x, int y );
	virtual bool				canPitch();
	virtual void				pitch( float r );
	virtual bool				canYaw();
	virtual void				yaw( float r );
	virtual bool				canRoll();
	virtual void				roll( float r );
	
	virtual void				resetPosition( Vector3 & pos );
	virtual void				resetFrame();

	//----------------------------------------------------------------------------

	// Static
	static void					registerPort( const ClassKey &key, HICON icon ); 
	static void					unregisterPort( const ClassKey &key );

	static int					portCount();
	static Factory *			port( int n );
	static const char *			portName( int n );

	static Factory *			port( const ClassKey &key );
	static HICON				portIcon( const ClassKey &key );
	static int					portIndex( Factory *pFactory );
	static int					portIndex( const ClassKey & key );

	// Data
	WidgetKey					m_NodeKey;			// WidgetKey assigned to the created node
	ClassKey					m_Type;
	ClassKey					m_Class;
	CharString					m_Name;
	BaseNode::Ref				m_pTemplate;		// template from which the node will be created..
	Array<Ref>					m_Children;
	
	CDocument *					m_Parent;			// pointer to the ScenePort object
	bool						m_Update;

	BaseNode::Ref				m_pCachedNode;		// cached version of our node

	// Static
	static Array<Factory *>		s_NodePorts;
	static Hash<ClassKey, HICON>
								s_NodeIcons;

protected:
	//{{AFX_MSG(Port)
	//}}AFX_MSG
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	DECLARE_MESSAGE_MAP()

	BOOL						DoFileSave();
};

//---------------------------------------------------------------------------------------------------

inline BaseNode * BaseNodePort::getCachedNode()
{
	return m_pCachedNode;
}

//---------------------------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------
// EOF
