/*
	Factory.h

	Factory class, creates Widget objects and contains reflection information about the class
	it contains.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef FACTORY_H
#define FACTORY_H

#include "Debug/Trace.h"
#include "File/Stream.h"
#include "Standard/CharString.h"
#include "Standard/Hash.h"
#include "Standard/Array.h"
#include "Standard/CriticalSection.h"

#include "ClassKey.h"
#include "Widget.h"

#include <vector>
#include <map>

#include "MedusaDll.h"

//----------------------------------------------------------------------------

#pragma warning(disable:4251)		// dll-interface warning for template classes, idiots!
class Widget;						// forward declare Widget, see Widget.h

const ClassKey NULL_CLASS( (qword)0 );

//----------------------------------------------------------------------------

class DLL Factory
{
public:
	// Exceptions
	class FactoryAlreadyExists {};
	class FactoryDoesntExist {};

	// Construction
	Factory( const char * a_pName, Factory * a_pBase, bool a_bSerializeKey, int a_nTypeId );
	virtual ~Factory();

	// Accessors
	const ClassKey &			classKey() const;							// unique ID for our class
	const char *				className() const;							// name for our class
	const ClassKey &			typeKey() const;							// class key of our base class
	bool						serializeKey() const;
	bool						isType( const ClassKey & nType ) const;

	int							childCount() const;							// number of child factories
	Factory *					child( int i ) const;						// get child i

	// Mutators
	void						setSerializeKey( bool a_bSerializeKey );
	virtual Widget *			createWidget() = 0;
	virtual Widget *			createWidget( const InStream &input ) = 0;
	virtual bool				isAbstract() = 0;

	// Static
	static bool					loadWidget( const InStream &input, Widget * & pWidget, const ClassKey & type, bool bLoadInPlace );
	static bool					saveWidget( const OutStream & output, Widget * pWidget );
	static Factory *			findFactory( const ClassKey &key );
	static Factory *			findFactoryByName( const char * pName );
	static Factory *			findFactoryByTypeId( int nTypeId );
	static Widget *				createWidget( const ClassKey &key );
	static Widget *				createNamedWidget( const char * pName );	// creates an object by name... finds the first concrete factory
	static const char *			className( const ClassKey & key );
	static const ClassKey &		classType( const ClassKey & key );

	static int					typeCount( const ClassKey & key );
	static const ClassKey &		type( const ClassKey &key, int n );
	static bool					isType( const ClassKey & type, const ClassKey & check );		// returns true if "check" is a class derived from "type"

	// This functions are used to associate a ClassKey with a factory object. Currently, factories
	// do this inside their constructor automatically, however to support legacy ClassKey generation
	// this function is needed, see the "RegisterFactory" class.
	static void					registerFactory( const ClassKey & key, Factory * pFactory );
	static void					unregisterFactory( const ClassKey & key );

	//! This function initializes the ClassKey storage in the given dictionary object. This allows us to send class ID's using shorter ID
	//! values instead of the full 64-bit values.
	static bool					initializeClassDictionary( Dictionary * a_pDictionary );

private:
	// Types
	typedef std::map< ClassKey, Factory *>		FactoryHash;
	typedef std::map< int, Factory *>			TypeIdHash;
	typedef std::vector< Factory * >			ChildFactories;

	// Data
	const char *				m_pName;			// class name
	Factory *					m_pBase;			// base factory
	bool						m_bSerializeKey;	// serialize our UniqueID 
	int							m_nTypeId;			// our typeId
	
	ClassKey					m_nKey;				// our class key
	ChildFactories				m_Children;			// classes derived from this class

	// Static
	static FactoryHash &		GetFactoryHash();
	static CriticalSection &	GetFactoryLock();
	static TypeIdHash &			GetTypeIdHash();
};

//----------------------------------------------------------------------------

inline const ClassKey & Factory::classKey() const
{
	return m_nKey;
}

inline const char * Factory::className() const
{
	return m_pName;
}

inline const ClassKey & Factory::typeKey() const
{
	return m_pBase != NULL ? m_pBase->classKey() : NULL_CLASS;
}

inline bool Factory::serializeKey() const
{
	return m_bSerializeKey;
}

inline bool Factory::isType( const ClassKey & nType ) const
{
	if ( nType == m_nKey )
		return true;
	if ( m_pBase != NULL )
		return m_pBase->isType( nType );

	return false;
	//return isType( nType, classKey() );
}

inline int Factory::childCount() const
{
	return (int)m_Children.size();
}

inline Factory * Factory::child( int i ) const
{
	if ( i >= 0 && i < (int)m_Children.size() )
		return m_Children[ i ];
	return NULL;
}

//---------------------------------------------------------------------------------------------------

inline void Factory::setSerializeKey( bool a_bSerializeKey )
{
	m_bSerializeKey = a_bSerializeKey;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream &input, T * & pWidgetT )
{
	Widget * pWidget = pWidgetT;
	if (! Factory::loadWidget( input, pWidget, T::classKey(), false ) )
		throw File::FileError();
	pWidgetT = (T *)pWidget;
	return input;
}

template<class T>
inline const InStream & operator>>( const InStream & input, T & object )
{
	Widget * pWidget = &object;
	if (! Factory::loadWidget( input, pWidget, T::classKey(), true ) )
		throw File::FileError();

	return input;
}

inline const OutStream & operator<<( const OutStream & output, const Widget * pWidget )
{
	if (! Factory::saveWidget( output, const_cast<Widget *>( pWidget ) ) )
		throw File::FileError();

	return output;
}

inline const OutStream & operator<<( const OutStream & output, const Widget & object )
{
	Widget * pWidget = const_cast<Widget *>( &object );
	if (! Factory::saveWidget( output, pWidget ) )
		throw File::FileError();

	return output;
}

#endif

//----------------------------------------------------------------------------------------
// EOF
