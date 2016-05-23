/*
	BaseNodePort.cpp
	(c)2005 Palestar, Richard Lyle
*/

#pragma warning( disable: 4786 )	// identifier was truncated to '255' characters in the browser information

#include "StdAfx.h"
#include "Port.h" 
#include "ChildFrame.h"
#include "BaseNodePort.h"

#include "Tools/ResourcerDoc/ResourcerDoc.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( BaseNodePort, Widget );
REGISTER_FACTORY_KEY(  BaseNodePort, 3932872277263557298 );

BEGIN_PROPERTY_LIST( BaseNodePort, Widget )
	ADD_NOEDIT_PROPERTY( m_NodeKey );
	ADD_PROPERTY_FLAGS( m_NodeKey, PF_NOCOPY );
	ADD_CLASSKEY_PROPERTY( m_Class, BaseNode );
	ADD_PROPERTY( m_Name );
	ADD_PROPERTY( m_pTemplate );
	ADD_NOEDIT_PROPERTY( m_Children );
END_PROPERTY_LIST()

Array<Factory *>						BaseNodePort::s_NodePorts;
Hash<ClassKey, HICON>					BaseNodePort::s_NodeIcons;

//----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(BaseNodePort, CDocument)
	//{{AFX_MSG_MAP(Port)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
END_MESSAGE_MAP()

BaseNodePort::BaseNodePort() : Widget()
{
	m_Type = CLASS_KEY(BaseNode);
	m_Class = CLASS_KEY(BaseNode);
	m_Name = "Node";
	m_Parent = NULL;
	m_Update = true;
	m_bAutoDelete = false;

	SetTitle( m_Name );
}

BaseNodePort::~BaseNodePort()
{
	// destroy all views of this document
	POSITION pos = GetFirstViewPosition();   
	while (pos != NULL)   
	{
		GetNextView(pos)->GetParentFrame()->DestroyWindow();
		pos = GetFirstViewPosition();
	}
}

//---------------------------------------------------------------------------------------------------

bool BaseNodePort::copy( void * pLeft, Type * pLeftType, const void * pRight, Type * pRightType, 
						Property * pProperty, int nElement, bool bMutator )
{
	bool bCopied = PropertyClass::copy( pLeft, pLeftType, pRight, pRightType, pProperty, nElement, bMutator );
	if ( bMutator && bCopied )
		m_Update = true;

	return bCopied;
}

//-------------------------------------------------------------------------------

const int VERSION_031203 = 31203;

bool BaseNodePort::read( const InStream & input )
{
	if (! Widget::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_031203:
			input >> m_Class;
			input >> m_Name;
			input >> m_Children;
			break;
		default:
			input.file()->setPosition( input.file()->position() - sizeof(int) );
			m_Class = CLASS_KEY(BaseNode);
			input >> m_Name;
			input >> m_Children;
			break;
		}

		SetTitle( m_Name );
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------

bool BaseNodePort::update() const
{
	return( m_Update );
}

const char * BaseNodePort::name() const
{
	return( m_Name );
}

int BaseNodePort::childCount() const
{
	return( m_Children.size() );
}

BaseNodePort * BaseNodePort::child(int n) const
{
	return( m_Children[ n ] );
}

CDocument * BaseNodePort::parent() const
{
	return( m_Parent );
}

//----------------------------------------------------------------------------

void BaseNodePort::setUpdate( bool bUpdate /*= true*/, bool bRecursive /*= false*/ )
{
	m_Update = bUpdate;
	if ( bRecursive )
	{
		for(int i=0;i<m_Children.size();i++)
			m_Children[i]->setUpdate( bUpdate, bRecursive );
	}
}

void BaseNodePort::setName( const char * name )
{
	m_Name = name;
}

void BaseNodePort::addChild( BaseNodePort * pNode )
{
	m_Children.push( pNode );
}

void BaseNodePort::removeChild( int n )
{
	m_Children.remove( n );
}

void BaseNodePort::setParent( CDocument * pParent )
{
	m_Parent = pParent;

	for(int i=0;i<m_Children.size();i++)
		m_Children[i]->setParent( pParent );
}

void BaseNodePort::newKey( bool recursive /*= true*/ )
{
	m_NodeKey = WidgetKey();

	if (recursive )
		for(int i=0;i<m_Children.size();i++)
			m_Children[i]->newKey( recursive );
}

//----------------------------------------------------------------------------

bool BaseNodePort::isChild( BaseNodePort *node ) const
{
	if ( node == this )
		return( true );

	for(int i=0;i<m_Children.size();i++)
		if ( m_Children[i]->isChild( node ) )
			return( true );

	return( false );
}

void BaseNodePort::updateParent() const
{ 
	if ( m_Parent != NULL )
		m_Parent->UpdateAllViews(NULL);
}

void BaseNodePort::updateAllViews() 
{
	// set the update flag, so the parent will rebuild this node
	m_Update = true;
	// call the CDocument update
	UpdateAllViews( NULL );
	// if we have a valid parent, tell the parent to update it's view
	if ( m_Parent != NULL )
		m_Parent->UpdateAllViews( NULL );
}

BaseNodePort * BaseNodePort::findNode( const WidgetKey & key )
{
	if ( m_NodeKey == key )
		return( this );

	BaseNodePort * found = NULL;
	for(int i=0;i<m_Children.size();i++)
		if ( (found = m_Children[i]->findNode( key )) != NULL )
			return( found );

	return( NULL );
}

BaseNodePort * BaseNodePort::findNode( const char * name )
{
	if ( m_Name == name )
		return( this );

	BaseNodePort * found = NULL;
	for(int i=0;i<m_Children.size();i++)
		if ( (found = m_Children[i]->findNode( name )) != NULL )
			return( found );

	return( NULL );
}

BaseNode * BaseNodePort::findCachedNode( const char * pName )
{
	BaseNodePort * pPort = findNode( pName );
	if ( pPort != NULL )
		return pPort->getCachedNode();
	return NULL;
}

BaseNode * BaseNodePort::findCachedNode( const WidgetKey & nKey )
{
	BaseNodePort * pPort = findNode( nKey );
	if ( pPort != NULL )
		return pPort->getCachedNode();
	return NULL;
}

BaseNode * BaseNodePort::updateNodeCache( bool bForceUpdate /*= false */, bool bCheckKeys /*= false*/ )
{
	// detach all children
	if ( m_pCachedNode.valid() )
		m_pCachedNode->detachAllNodes();

	while( bCheckKeys && !CResourcerDoc::document()->checkKeyUnique( m_NodeKey ) )
	{
		report( "Key conflict detected." );
		newKey( false );

		// force our port to be saved..
		((ScenePort *)m_Parent)->m_bKeyConflicts = true;
	}

	// only rebuild the node if we have no cached version, or the node has been modified
	if ( !m_pCachedNode.valid() || m_Update || bForceUpdate )
	{
		m_pCachedNode = createNode();
		m_Update = false;

		if ( !m_pCachedNode.valid() )
			return NULL;
		//ASSERT( pNode->key() == key() );
	}

	// update children
	for(int i=0;i<m_Children.size();i++)
	{
		BaseNodePort::Ref pChildPort = m_Children[i];
		if (! pChildPort.valid() )
		{
			report( CharString().format( "Child %d is invalid!", i) );
			continue;
		}

		BaseNode::Ref pChild = m_Children[i]->updateNodeCache( bForceUpdate, bCheckKeys );
		if (! pChild.valid() )
		{
			report( CharString().format( "Child %d returned a invalid node!", i ) );
			continue;
		}

		if ( pChild->parent() != NULL )
			pChild->detachSelf();
		m_pCachedNode->attachNode( pChild );
	}

	return m_pCachedNode;
}

void BaseNodePort::recurseDependencies( DependentArray & dep )
{
	// get the dependents for this node
	dependencies( dep );
	// update the children nodes
	for(int i=0;i<m_Children.size();i++)
		m_Children[i]->recurseDependencies( dep );
}

void BaseNodePort::report( const char * pMessage )
{
	ScenePort * pParent = dynamic_cast<ScenePort *>( parent() );
	if ( pParent != NULL )
		pParent->report( CharString().format( "%s ; %s", name(), pMessage ) );
	else
		MessageBox( NULL, pMessage, name(), MB_OK );
}

//----------------------------------------------------------------------------

bool BaseNodePort::import( const char * pFileName )
{
	return false;
}

void BaseNodePort::dependencies( DependentArray & dep )
{}

CFrameWnd * BaseNodePort::createView()
{
	return NULL;
}

BaseNode * BaseNodePort::createNode()
{
	// check the class, make sure it's of the correct type
	if (! Factory::isType( m_Type, m_Class ) )
	{
		if ( Factory::isType( CLASS_KEY(BaseNode), m_Type ) )
			m_Class = m_Type;
		else if ( Factory::isType( CLASS_KEY(BaseNode), m_Class ) )
			m_Type = m_Class;
	}

	if ( !m_pTemplate.valid() || m_pTemplate->factory()->classKey() != m_Class )
	{
		Factory * pFactory = Factory::findFactory( m_Class );
		if ( pFactory != NULL )
		{
			// make sure the class key contained in the port isn't a deprecated class key
			// go ahead and update it anytime we update the template object, otherwise
			// we'll keep re-creating the template each time we load the object.
			m_Class = pFactory->classKey();

			Widget * pUncasted = pFactory->createWidget();
			if ( pUncasted != NULL )
			{
				BaseNode * pNode = WidgetCast<BaseNode>( pUncasted );
				if ( pNode != NULL )
					m_pTemplate = pNode;

				if (! m_pTemplate.valid() )
					delete pUncasted;
			}
		}
	}

	if ( m_pTemplate.valid() )
	{
		BaseNode * pCopy = (BaseNode *)m_pTemplate->copy();
		initializeNode( pCopy );
		return pCopy;
	}

	return NULL;
}

void BaseNodePort::initializeNode( BaseNode * pNode )
{
	if ( pNode != NULL )
	{
		pNode->setKey( m_NodeKey );
		pNode->setName( name() );
	}
}

bool BaseNodePort::canMove()
{
	return false;
}

void BaseNodePort::moveXY( int x, int y )
{}

void BaseNodePort::move( Vector3 & movexy )
{}

bool BaseNodePort::canScale()
{
	return false;
}

void BaseNodePort::scale( int x, int y )
{}

bool BaseNodePort::canPitch()
{
	return false;
}

void BaseNodePort::pitch( float r )
{}

bool BaseNodePort::canYaw()
{
	return false;
}

void BaseNodePort::yaw( float r )
{}

bool BaseNodePort::canRoll()
{
	return false;
}

void BaseNodePort::roll( float r )
{}

void BaseNodePort::resetPosition( Vector3 & pos )
{}

void BaseNodePort::resetFrame()
{}

//----------------------------------------------------------------------------

void BaseNodePort::registerPort( const ClassKey &key, HICON icon )
{
	// limit the size because we only have so much toolbar space
	if ( s_NodePorts.size() < MAX_NODE_PORTS)
	{
		s_NodePorts.push( Factory::findFactory( key ) );
		s_NodeIcons[ key ] = icon;
	}
}

void BaseNodePort::unregisterPort( const ClassKey &key )
{
	s_NodePorts.removeSearch( Factory::findFactory( key ) );
	s_NodeIcons.remove( key );
}

int BaseNodePort::portCount()
{
	return( s_NodePorts.size() );
}

Factory * BaseNodePort::port( int n ) 
{
	return( s_NodePorts[ n ] );
}

const char * BaseNodePort::portName( int n )
{
	return( port( n )->className() );
}

Factory * BaseNodePort::port( const ClassKey &key ) 
{
	return( Factory::findFactory( key ) );
}

HICON BaseNodePort::portIcon( const ClassKey &key ) 
{
	return( s_NodeIcons[ key ] );
}

int BaseNodePort::portIndex( Factory *pFactory ) 
{
	return( s_NodePorts.search( pFactory ) );
}

int BaseNodePort::portIndex( const ClassKey & key )
{
	return( s_NodePorts.search( port( key ) ) );
}

//-------------------------------------------------------------------------------

void BaseNodePort::OnFileSave()
{
	BaseNodePort * pDoc = dynamic_cast<BaseNodePort *>( this );
	ASSERT( pDoc );
	
	Port * pParent = dynamic_cast<Port *>( pDoc->m_Parent );
	if ( pParent != NULL )
	{
		pDoc->UpdateAllViews(NULL,true,NULL);
		pParent->OnFileSave();
	}
	else
		MessageBox(NULL,_T("Invalid parent document, cannot save!"),_T("Error"), MB_OK );
}

void BaseNodePort::OnFileSaveAs()
{
	BaseNodePort * pDoc = dynamic_cast<BaseNodePort *>( this );
	ASSERT( pDoc );
	
	Port * pParent = dynamic_cast<Port *>( pDoc->m_Parent );
	if ( pParent != NULL )
	{
		pDoc->UpdateAllViews(NULL,true,NULL);
		pParent->OnFileSaveAs();
	}
	else
		MessageBox(NULL,_T("Invalid parent document, cannot save!"),_T("Error"), MB_OK );
}

BOOL BaseNodePort::DoFileSave()
{
	OnFileSave();
	SetModifiedFlag( false );
	return true;
}

//----------------------------------------------------------------------------
// EOF
