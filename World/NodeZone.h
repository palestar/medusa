/*
	NodeZone.h

	This node is used to partition an area of the worldContext into a manageable chunk, it is also used 
	to deal with scale issues in a very large worldContext.

	- No collision tests are done with objects in another zone
	- This zone provides BSP based collision detection

	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef NODEZONE_H
#define NODEZONE_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Scene.h"
#include "CollisionHashAbstract.h"
#include "Constants.h"
#include "Noun.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

// forward declare
class Noun;
class WorldContext;

class DLL NodeZone : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef CollisionHashAbstract::NounCollision	NounCollision;

	typedef WidgetReference< NodeZone >		wRef;
	typedef Reference< NodeZone >			Ref;

	// Construction
	NodeZone();
	~NodeZone();

	// Widget interface
	bool				read( const InStream & input );
	bool				write( const OutStream & output );

	// BaseNode interface
	void				onChildDetached();
	BoxHull				hull() const;
	
	void				preRender( RenderContext &context, 
							const Matrix33 & frame, const Vector3 & position );

	// Accessors
	bool				locked() const;				// is this zone current locked by the WorldContext

	Scene *				scenery( bool a_bBlocking = true ) const;
	WorldContext *		context() const;			// get the WorldContext this zone is attached too
	dword				tick() const;	

	int					nounCount() const;

	BoxHull				worldHull() const;			// returns a BoxHull for this zone in world space
	qword				profileTime() const;		// amount of CPU used updating this zone

	// Mutators
	void				initializeZone( WorldContext * pContext );
	void				releaseZone();

	void				setLocked( bool bLocked );
	void				setScenery( Scene * pScenery );
	void				setHull( const BoxHull & hull );

	void				setContext( WorldContext * pContext );
	void				setTick( dword tick );		// set the tick for all nouns in this zone

	void				addProfileTime( qword nTime );
	void				setProfileTime( qword nTime /*= 0*/ );

	bool				attachNoun( Noun * pNoun, BaseNode * pParent = NULL );
	bool				replaceNoun( Noun * pReplace, Noun * pWith );
	void				detachNoun( Noun * pNoun );
	void				leaveZone( Noun * pNoun );

	bool				transferNoun( Noun * pNoun, 
							NodeZone * pNewZone, 
							bool updatePosition );

	virtual void		onAttachNoun( Noun * pNoun );				// attach the root noun into the update chain
	virtual void		onDetachNoun( Noun * pNoun );				// detach the root noun from the update chain

	void				onWorldTransform( const Vector3 & vAdd );	// add a vector to this zones current position
	void				simulate( dword nTick );					// update all nouns in this zone
	void				close();									// close this zone, called before the zone is deleted

	bool				proximityCheck( const Vector3 & origin, 
							float fRadius, Array< NounCollision > & nouns );
	bool				proximityCheck( const Vector3 & origin, 
							float fRadius, Array< NounCollision > & nouns,
							const ClassKey & nType );
	
	// Helpers
	bool				isActiveZone() const;						// is this the active zone
	bool				inZone( Noun * pNoun ) const;				// is this noun attached to this zone

	// Static
	static void			setContext( BaseNode * pNode, 
							WorldContext * pContext );			// set the context pointer for all nouns in the heirarchy
	static void			setZone( BaseNode * pNode, 
							NodeZone * pZone );					
	static void			setTick( BaseNode * pNode, 
							dword tick );							// set the tick for all nouns in heirarchy
	static void			updateLastPosition( BaseNode * pNode );

protected:
	// Data
	Scene::Link			m_Scenery;			// scenery for this zone
	BoxHull				m_Hull;				// our hull

	// non-serialized
	bool				m_bLocked;			// is this zone current locked by the context
	WeakReference< WorldContext >
						m_pContext;			// our WorldContext
	dword				m_Tick;				// current tick


	CollisionHashAbstract *		
						m_pCollisionHash;	// our collision hash

	int					m_nNounCount;		// how many nouns in this zone
	qword				m_nProfileTime;		// CPU usage

	virtual void		detectCollisions();
	virtual bool		hookNouns( BaseNode::Ref pNode );				// attach all nouns to update chain
	virtual void		unhookNouns( BaseNode::Ref pNode );			// detach all nouns from update chain
};

//----------------------------------------------------------------------------

inline bool NodeZone::locked() const
{
	return m_bLocked;
}

inline Scene * NodeZone::scenery( bool a_bBlocking /*= true*/ ) const
{
	return m_Scenery.pointer( true, a_bBlocking );
}

inline WorldContext * NodeZone::context() const
{
	return m_pContext;
}

inline dword NodeZone::tick() const
{
	return m_Tick;
}

inline int NodeZone::nounCount() const
{
	return m_nNounCount;
}

inline BoxHull NodeZone::worldHull() const
{
	if ( m_Hull.isValid() )
		return BoxHull( m_Hull, position() );

	return BoxHull( false );		// return a zero sized box
}

inline qword NodeZone::profileTime() const
{
	return m_nProfileTime;
}

//----------------------------------------------------------------------------

inline void NodeZone::addProfileTime( qword nTime ) 
{
	m_nProfileTime += nTime;
}

inline void NodeZone::setProfileTime( qword nTime /*= 0*/ )
{
	m_nProfileTime = nTime;
}

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF
