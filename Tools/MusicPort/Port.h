/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef MUSICPORT_H
#define MUSICPORT_H

#include "Standard/Dll.h"
#include "Tools/ResourcerDoc/Port.h"

#include "Audio/Music.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef MUSICPORT_DLL
#define DLL		DLL_EXPORT
#else
#define DLL		DLL_IMPORT
#endif

//-------------------------------------------------------------------------------

class DLL MusicPort : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<MusicPort>	Ref;
	typedef Music::Segment			Segment;
	typedef Music::Branch			Branch;

	// Construction
	MusicPort();

	// Widget Interface
	bool						read( const InStream & input );
	// Port interface
	void						dependencies( DependentArray & dep );		// get the depdendencies
	Resource::Ref				createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program

	// Data
	CharString					m_WaveFile;
	int							m_BufferSize;

	Array< CharString >	m_SegmentId;
	Array< Segment >	m_Segments;

	Array< Array< Branch > >
								m_Branches;
	Array< CharString >
								m_Triggers;
};

#endif

//-------------------------------------------------------------------------------
// EOF
