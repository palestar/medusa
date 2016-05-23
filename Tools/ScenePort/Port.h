/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef SCENEPORT_H
#define SCENEPORT_H

#pragma warning( disable: 4786 )	// identifier was truncated to '255' characters in the browser information

#include <set>

#include "Render3d/Scene.h" 
#include "Render3d/Material.h"
#include "Tools/ResourcerDoc/Port.h"
#include "NodePort.h"
#include "ScenePortDll.h"

//----------------------------------------------------------------------------

struct Segment
{
	CharString	id;
	float				beginTime;
	float				endTime;
	
	Segment &		operator=(const Segment & rhs );
};

struct Flag
{
	CharString	id;
	dword				bit;

	Flag &			operator=(const Flag & rhs );
};

//----------------------------------------------------------------------------

class DLL ScenePort : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	//! Types
	typedef Reference<ScenePort>	Ref;

	class DLL Importer 
	{
	public:
		Importer();
		~Importer();

		//! Try to import the given file into the Scene, return true on success!
		virtual bool		import( ScenePort * a_pScenePort, const char * a_pFile ) = 0;
	};
	typedef std::set< Importer * >		ImporterSet;

	// Construction
	ScenePort();
	// Widget Interface
	bool						read( const InStream & input );
	// Port interface
	void						dependencies( DependentArray & dep );		// get the depdendencies
	Resource::Ref				createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program

	// Accessors
	BaseNode *					findCachedNode( const WidgetKey & key ) const;	// find node by key in node cache
	BaseNode *					findCachedNode( const char * name ) const;		

	// Data
	BaseNodePort::Ref			m_Root;
	float						m_Fps;
	dword						m_Bits;

	Vector3						m_TargetPosition;			// what is the camera looking at
	float						m_TargetDistance;			// distance of camera from target
	Matrix33					m_TargetFrame;				// frame of target

	Array< Segment >			m_Segments;

	// View Data
	BaseNodePort::Ref			m_pSelectedNode;			// selected node port
	int							m_CurrentSegment;			// set by CSegmentView
	bool						m_bKeyConflicts;			// set to true when key conflicts are detected, Port should be saved..

private:
	//! Data
	BaseNode::Ref				m_Nodes;
	//! Static Data
	static ImporterSet			sm_Importers;

	friend class BaseNodePort;
};

//-------------------------------------------------------------------------------

inline const InStream & operator>>(const InStream & input, Segment &segment )
{
	input >> segment.id;
	input >> segment.beginTime;
	input >> segment.endTime;
	return input;
}

inline const OutStream & operator<<(const OutStream & output, const Segment &segment )
{
	output << segment.id;
	output << segment.beginTime;
	output << segment.endTime;
	return output;
}

inline Segment & Segment::operator=(const Segment & rhs )
{
	id = rhs.id;
	beginTime = rhs.beginTime;
	endTime = rhs.endTime;
	return *this;
}

//----------------------------------------------------------------------------

inline const InStream & operator>>(const InStream & input, Flag &flag )
{
	input >> flag.id;
	input >> flag.bit;
	return input;
}

inline const OutStream & operator<<(const OutStream & output, const Flag &flag )
{
	output << flag.id;
	output << flag.bit;
	return output;
}

inline Flag & Flag::operator=(const Flag & rhs )
{
	id = rhs.id;
	bit = rhs.bit;
	return *this;
}


#endif

//-------------------------------------------------------------------------------
// EOF
