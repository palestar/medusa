/*
	NounContext.h

	This is the container object for a Noun's static data. 
	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef NOUNCONTEXT_H
#define NOUNCONTEXT_H

#include "Resource/ResourceLink.h"
#include "System/LocalizedString.h"
#include "Reflection/Value.h"
#include "Render3D/Scene.h"
#include "Constants.h"
#include "WorldDll.h"

class Noun;				// forward declare

//----------------------------------------------------------------------------

class DLL NounContext : public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef ResourceLink< NounContext >			Link;
	typedef Reference< NounContext >			Ref;
	typedef Tree< CharString, Resource::Link >	Resources;
	typedef Tree< CharString, Value >			Attributes;

	//! This is the base class any type of static data that is needed by the noun classes.
	class DLL Data : public Widget
	{
	public:
		DECLARE_WIDGET_CLASS();
		DECLARE_PROPERTY_LIST();

		//! Types
		typedef Reference< Data >		Ref;

		//! Construction
		Data( const char * a_pID = NULL );
		//! Accessors
		const CharString &	id() const;
		//! Mutators
		void				setId( const char * a_pID );

	protected:
		//! Data
		CharString			m_sID;
	};
	typedef Array< Data::Ref >			DataArray;

	//! generic wrapper for a resource 
	class DLL ResourceData : public Data
	{
	public:
		DECLARE_WIDGET_CLASS();
		DECLARE_PROPERTY_LIST();

		//! Construction
		ResourceData();
		ResourceData( const char * a_pID, const char * a_pPort );
		//! Widget interface
		bool				read( const InStream & input );
		//! Accessors
		const CharString &	port() const;
		Resource *			resource( bool a_bBlocking = true ) const;
		//! Mutators
		void				unload();			// unloads the loaded resource from memory
	protected:
		//! Data
		CharString			m_sPort;
		Resource::Link		m_pResource;
	};

	// Construction
	NounContext();
	NounContext( Scene * pScene );
	NounContext( bool isStatic );

	// Accessors
	const ClassKey &	type() const;			// noun class key
	const char *		name() const;			// the noun name
	const char *		description() const;	// full description of this noun
	Scene *				scene( bool a_bBlocking = true ) const;			// noun scene
	const BoxHull &		hull() const;			// box hull of the scene
	float				mass() const;			// mass of this object
	dword				flags() const;			// user flags

	int					dataCount() const;
	Data *				data( int n ) const;

	//! This function returns the first data object of the given type, if a_pID is not NULL
	//! Then it will check the ID of the data as well.
	template<typename T>
	T *					findData( const char * a_pID = NULL ) const;
	//! This function will fill the array with all data of the given type, it returns the total number found.
	template<typename T>
	int					findData( Array<T *> a_Data ) const;
	//! This functions returns Resource * for the resource with the given ID, this is the older
	//! method used to link resources to a noun. Newer code should just 
	Resource *			resource( const char * pName,
							bool a_bBlocking = true ) const;

	//! This functions creates a noun as specified by this context.
	Noun *				createNoun() const;		

	// Mutators
	void				setType( const ClassKey & nKey );
	void				setName( const char * name );
	void				setDescription( const char * description );
	void				setScene( Scene::Link pScene, bool a_bCalculateHull = true );
	void				setMass( float mass );
	void				setHull( const BoxHull & a_Hull );
	void				setFlags( dword flags );
	void				addData( Data * a_pData );

	// deprecated...
	class DLL Drop : public Widget	
	{
	public:
		DECLARE_WIDGET_CLASS();
		DECLARE_PROPERTY_LIST();

		typedef Reference< Drop >		Ref;

		Drop() : m_nDropClassKey( (qword)0 ), m_nBaseAmount( 0 ), m_nRandomAmount( 0 ), m_nChance( 0 )
		{}

		ClassKey			m_nDropClassKey;
		int					m_nBaseAmount;
		int					m_nRandomAmount;
		int					m_nChance;
	};

protected:
	// Data
	ClassKey			m_Type;			
	Reference< Noun >	m_pTemplate;			// noun template object
	CharString			m_Name;
	CharString			m_Description;
	Scene::Link			m_Scene;
	BoxHull				m_Hull;
	float				m_Mass;
	dword				m_Flags;
	DataArray			m_Data;
};

//---------------------------------------------------------------------------------------------------

inline NounContext::Data::Data( const char * a_pID /*= NULL*/ ) : m_sID( a_pID != NULL ? a_pID : "" )
{}

inline const CharString & NounContext::Data::id() const
{
	return m_sID;
}

inline void NounContext::Data::setId( const char * a_pID )
{
	m_sID = a_pID;
}

//---------------------------------------------------------------------------------------------------

inline NounContext::ResourceData::ResourceData() 
{}

inline NounContext::ResourceData::ResourceData( const char * a_pID, const char * a_pPort ) : 
	Data( a_pID ), m_sPort( a_pPort )
{
	m_pResource.setKey( StringHash64( m_sPort ).hash() );
}

inline bool NounContext::ResourceData::read( const InStream & input )
{
	if (! Data::read( input ) )
		return false;
	m_pResource.setKey( StringHash64( m_sPort ).hash() );
	return true;
}


inline const CharString & NounContext::ResourceData::port() const
{
	return m_sPort;
}

inline Resource * NounContext::ResourceData::resource( bool a_bBlocking /*= true*/ ) const
{
	return m_pResource.pointer( true, a_bBlocking );
}

inline void NounContext::ResourceData::unload()
{
	m_pResource.unload();
}

//----------------------------------------------------------------------------

inline const ClassKey & NounContext::type() const
{
	return m_Type;
}

inline const char * NounContext::name() const
{
	return m_Name;
}

inline const char * NounContext::description() const
{
	return m_Description;
}

inline Scene * NounContext::scene( bool a_bBlocking /*= true*/ ) const
{
	return m_Scene.pointer( true, a_bBlocking );
}

inline const BoxHull & NounContext::hull() const
{
	return m_Hull;
}

inline float NounContext::mass() const
{
	return m_Mass;
}

inline dword NounContext::flags() const
{
	return m_Flags;
}

inline int NounContext::dataCount() const
{
	return m_Data.size();
}

inline NounContext::Data * NounContext::data( int n ) const
{
	return m_Data[ n ];
}

template<typename T>
inline T * NounContext::findData( const char * a_pID /*= NULL */ ) const
{
	for(int i=0;i<m_Data.size();++i)
	{
		T * pData = WidgetCast<T>( m_Data[i] );
		if ( pData != NULL && (a_pID == NULL || pData->id() == a_pID) )
			return pData;
	}
	return NULL;
}

template<typename T>
int NounContext::findData( Array<T *> a_Data ) const
{
	for(int i=0;i<m_Data.size();++i)
	{
		T * pData = WidgetCast<T>( m_Data[i] );
		if ( pData != NULL )
			a_Data.push( pData );
	}

	return a_Data.size();
}

inline Resource * NounContext::resource( const char * pName, bool a_bBlocking /*= true*/ ) const
{
	ResourceData * pData = findData<ResourceData>( pName );
	if ( pData != NULL )
		return pData->resource( a_bBlocking );

	return NULL;
}

//----------------------------------------------------------------------------

inline void NounContext::setType( const ClassKey & nKey )
{
	m_Type = nKey;
}

inline void NounContext::setName( const char * name )
{
	m_Name = name;
}

inline void NounContext::setDescription( const char * description )
{
	m_Description = description;
}

inline void NounContext::setScene( Scene::Link pScene, bool a_bCalculateHull /*= true*/ )
{
	m_Scene = pScene;
	
	if ( a_bCalculateHull )
	{
		if ( m_Scene.valid() )
			m_Hull = m_Scene->calculateHull();
		else
			m_Hull = BoxHull( true );
	}
}

inline void NounContext::setMass( float mass )
{
	m_Mass = mass;
	if ( m_Mass < 1.0f )
		m_Mass = 1.0f;
}

inline void NounContext::setHull( const BoxHull & a_Hull )
{
	m_Hull = a_Hull;
}

inline void NounContext::setFlags( dword flags )
{
	m_Flags = flags;
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
