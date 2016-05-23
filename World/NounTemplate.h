/**
	@file NounTemplate.h
	@brief This class provides the container for the available noun types that the player can select...
	
	(c)2006 Palestar Inc
	@author Richard Lyle @date 10/16/2006 12:06:58 AM
*/

#ifndef NOUNTEMPLATE_H
#define NOUNTEMPLATE_H

#include "Render3D/Scene.h"
#include "Resource/Resource.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

class Noun;		// forward declare

class DLL NounTemplate : public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef ResourceLink< NounTemplate >		Link;
	typedef Reference< NounTemplate >		Ref;

	// Construction
	NounTemplate();

	// Widget interface
	bool				read( const InStream & );
	// Accessors
	dword				flags() const;			// required flags to select this noun
	int					rank() const;			// required rank for this noun
	Scene *				scene() const;			// get the scene containing our noun template

	// Mutators
	void				setFlags( dword nFlags );
	void				setRank( int rank );
	void				setScene( Scene::Link pScene );

	// helpers
	Noun *				noun();					// get the template object
	Noun *				spawn();				// spawn a copy of the template object

private:
	// Data
	dword				m_nFlags;
	int					m_nRank;
	Scene::Link			m_pScene;
};

//----------------------------------------------------------------------------

inline dword NounTemplate::flags() const
{
	return m_nFlags;
}

inline int NounTemplate::rank() const
{
	return m_nRank;
}

inline Scene * NounTemplate::scene() const
{
	return m_pScene;
}

//----------------------------------------------------------------------------

inline void NounTemplate::setFlags( dword nFlags )
{
	m_nFlags = nFlags;
}

inline void NounTemplate::setRank( int rank )
{
	m_nRank = rank;
}

inline void NounTemplate::setScene( Scene::Link pScene )
{
	m_pScene = pScene;
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF
