/*
	PropertyList.h
	Container class for a collection of class properties, see Standard/Property.h

	(c)2005 Palestar, Richard Lyle
*/

#ifndef PROPERTYLIST_H
#define PROPERTYLIST_H

#include "Reflection/Property.h"
#include "Reflection/TypeTemplate.h"
#include "Standard/Hash.h"
#include "Standard/Array.h"
#include "Standard/StringHelpers.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL PropertyList
{
public:
	// Types
	typedef Property::Find			Find;

	// Construction
	PropertyList( Type * pType, Type * pPointerType, PropertyList * pBaseClass );
	~PropertyList();

	// Accessors
	bool					initialized() const;							// is the list been initialized
																			// back to the original class pointer
	const char *			className() const;								// name of the class
	Type *					type() const;
	Type *					pointerType() const;

	PropertyList *			baseClass() const;								// base class for this property list
	int						childCount() const;								// number of PropertyList's using this object as their base
	PropertyList *			child( int n ) const;                     

	int						propertyCount() const;							// number of property objects
	Property *				property( int n ) const;

	int						find( const char * pName ) const;				// find a property by name
	int						findByNameId( dword nNameId ) const;			// find a property by name ID

	// Mutators
	void					initialize();									
	void					release();

	void					setClassName( const char * a_pName );
	int						addProperty( Property * pProp );
	void					removeProperty( int n );

	bool					addOption( const CharString & sProp, 
								const CharString & sName, 
								const CharString & sValue );

	bool					setFlags( const char * pName, dword nFlags );
	bool					addFlags( const char * pName, dword nFlags );
	bool					clearFlags( const char * pName, dword nFlags );
	bool					setNote( const char * pName, const char * pNote );

	bool					find( const Expression & e, 
								void * pThis, Find & found );				// find a property by expression
	bool					enumerate( void * pThis,
								Array< Find > & found );					// enumerate properties, returns all properties
	
	// Helpers

private:
	// Types
	typedef Array< Property::Ref >			Properties;
	typedef Array< PropertyList * >         Children;

	// Data
	bool					m_bInitialized;
	const char *			m_pName;
	PropertyList *			m_pBaseClass;
	Type::Ref				m_pType;
	Type::Ref				m_pPointerType;
	Children				m_Children;
	Properties				m_Properties;

   void						addChild( PropertyList * pChild );
   void						removeChild( PropertyList * pChild );
};

//---------------------------------------------------------------------------------------------------

inline bool PropertyList::initialized() const
{
	return m_bInitialized;
}

inline const char * PropertyList::className() const
{
	return m_pName;
}

inline Type * PropertyList::type() const
{
   return m_pType;
}

inline Type * PropertyList::pointerType() const
{
   return m_pPointerType;
}

inline PropertyList * PropertyList::baseClass() const
{
   return m_pBaseClass;
}

inline int PropertyList::childCount() const
{
   return (int)m_Children.size();
}

inline PropertyList * PropertyList::child( int n ) const
{
   return m_Children[ n ];
}

inline int PropertyList::propertyCount() const
{
	return (int)m_Properties.size();
}

inline Property * PropertyList::property( int n ) const
{
	return m_Properties[ n ];
}

inline int PropertyList::find( const char * pName ) const
{
	for(int i=0;i<(int)m_Properties.size();i++)
		if ( stricmp<char>( m_Properties[i]->name(), pName ) == 0)
			return i;

	return -1;
}

inline int PropertyList::findByNameId( dword nNameId ) const
{
	for(int i=0;i<m_Properties.size();++i)
		if ( m_Properties[i]->nameId() == nNameId )
			return i;

	return -1;
}

//---------------------------------------------------------------------------------------------------

class PropertyClass;

template<typename T>
class PropertyListTemplate : public PropertyList
{
public:
	PropertyListTemplate( PropertyList * pBaseClass ) 
		: PropertyList( TypeTemplate<T>::instance(), TypeTemplate<T *>::instance(), pBaseClass )
	{
		TypeCopyDefault<T *,T *>::instance();
		TypeCopyDefault< Reference<T>, T *>::instance();
		TypeCopyDefault< T *, Reference<T> >::instance();
	}
};

template<typename T>
class PropertyListAbstractTemplate : public PropertyList
{
public:
	PropertyListAbstractTemplate( PropertyList * pBaseClass ) 
      : PropertyList( TypeAbstract<T>::instance(), TypeAbstract<T *>::instance(), pBaseClass )
	{
		TypeCopyDefault<T *,T *>::instance();
		TypeCopyDefault< Reference<T>, T *>::instance();
		TypeCopyDefault< T *, Reference<T> >::instance();
	}
};

//---------------------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
