/*
	Property.h

	This class implements a generic interface for accessing a classes member data using a generic interface, 
	see PropertyClass.h
	
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PROPERTY_H
#define PROPERTY_H

#include "Standard/Expression.h"
#include "Reflection/Value.h"
#include "Standard/Reference.h"
#include "Standard/Array.h"
#include "Standard/String.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

enum PropertyFlags {

	PF_NOSAVE				= 0x00000001,			// property should not be saved
	PF_NOCOPY				= 0x00000002,			// property should not be copied
	PF_TRANSMIT				= 0x00000004,			// property should be transmitted over a network
	PF_UPDATE				= 0x00000008,			// property is changed and will be re-sent to clients who already have the object
	PF_RESOURCE				= 0x00000010,			// property is a resource link
	PF_OBJECT				= 0x00000040,			// property is an object (used for shallow serialization)
	PF_REPLICATE			= 0x00000080,			// property is replicated from the server down the clients
	PF_REPLICATE_UP			= 0x00000100,			// property is replicated from the client to the server
	PF_GLOBAL				= 0x00000200,			// Used with the PF_REPLICATE flag, this indicates the property should be sent to ALL clients
	PF_UDP					= 0X00000400,			// Used with the PF_REPLICATE flag, this indicates the data should be sent using UDP

	// Editor hint flags
	PF_NOEDIT				= 0x00010000,			// property cannot be edited
	PF_FILENAME				= 0x00020000,			// property is a file name
	PF_CLASSKEY				= 0x00040000,			// property is a class key
	PF_WIDGETKEY			= 0x00080000,			// property is a GID for another Widget
	PF_FLAGS				= 0x00100000,			// property is bit flags
	PF_ENUM					= 0x00209000,			// property is an enum
	PF_BITS					= 0x00400000,			// property is a bit field
	PF_COLOR				= 0x00800000,			// property is a color
	PF_PORT					= 0x01000000,			// property is a port filename
	PF_HOTKEY				= 0x02000000,			// property is a hot key
};

//----------------------------------------------------------------------------

class DLL Property : public Referenced
{
public:
	// Types
	typedef Reference< Property >	Ref;			// smart-pointer

	struct Option
	{
		CharString		sName;
		CharString		sValue;

		Option & operator=( const Option & copy );
	};
	struct Find
	{
		void *			pThis;
		Property *		pProperty;
		int				nElement;
		void *			pData;
		Type *			pType;
		CharString		sExpr;

		Find & operator=( const Find & copy );
	};
	class Container : public Referenced
	{
	public:
		typedef Reference< Container >		Ref;

	protected:
		virtual int		count( void * pThis ) const = 0;					// number of properties in this container
		virtual void *	pointer( void * pThis, int n ) const = 0;			// resolve property pointer

		virtual bool	isDynamic() const = 0;								// returns true if container is dynamic..
		virtual void	allocate( void * pThis, int nSize );				// resize a dynamic container
      	virtual int		add( void * pThis );							    // add a new item to a container, returns the index of the new item
      	virtual bool	remove( void * pThis, int n );						// remove an item from a container by it's index
      	virtual void	clear( void * pThis );								// remove all items from the container

		friend class Property;
	};

	// Construction
	Property( Container * pContainer, const char * pName, int nOffset, Type * pType, 
		dword nFlags = 0, const char * pNote = NULL);
	Property( const Property & copy );

	virtual ~Property();

	// Accessors
	Container *				container() const;			// container for this property
	dword					propertyId() const;			// unique id for this property
	const char *			name() const;				// property name
	dword					nameId() const;				// hash of the property name
	int						offset() const;				// byte offset
	Type *					type() const;				// property data type
	dword					flags() const;				// property flags
	const char *			note() const;				// property description

	int						optionCount() const;
	const Option &			option( int n ) const;


	// Mutators
	void					setPropertyId( dword nId );
	void					setFlags( dword nFlags );
	void					addFlags( dword nFlags );
	void					clearFlags( dword nFlags );
	void					setNote( const char * pNote );

	void				   	addOption( const CharString & name, const CharString & value );
	void					removeOption( int n );
	Option *				findOption( const char * pName );

	bool					enumerate( void * pThis, Array< Find > & found );
	bool					find( const Expression & e, void * pThis, Find & found );

	// Helpers
	int						size() const;			// size of property in bytes
	bool					isPointer() const;		// true if this property is a pointer
	bool					isReference() const;       // true if this property is a reference, isPointer() should be true as well

	int						count( void * pThis ) const;
	void *					containerPointer( void * pThis ) const;
	void *					pointer( void * pThis, int n ) const;			

	bool					isDynamic() const;         // is our container dynamic, if true then add(), remove(), and clear() can be called...
	void					allocate( void * pThis, int nSize );
	int						add( void * pThis );       
	bool					remove( void * pThis, int n );
	void					clear( void * pThis );

protected:
	// Data
	Container::Ref			m_pContainer;
	dword					m_nId;
	const char *			m_pName;
	dword					m_nNameId;
	int						m_nOffset;
	Type *					m_pType;
	dword					m_nFlags;
	const char *			m_pNote;

	Array< Option >			m_Options;
};

//----------------------------------------------------------------------------

inline Property::Container * Property::container() const
{
	return m_pContainer;
}

inline dword Property::propertyId() const
{
	return m_nId;
}

inline const char * Property::name() const
{
	return m_pName;
}

inline dword Property::nameId() const
{
	return m_nNameId;
}

inline int Property::offset() const
{
	return m_nOffset;
}

inline Type * Property::type() const
{
	return m_pType;
}

inline dword Property::flags() const
{
	return m_nFlags;
}

inline const char * Property::note() const
{
	return m_pNote;
}

inline int Property::optionCount() const
{
	return (int)m_Options.size();
}

inline const Property::Option & Property::option( int n ) const
{
	return m_Options[ n ];
}

//----------------------------------------------------------------------------

inline Property::Option * Property::findOption( const char * pName )
{
	for(int i=0;i<m_Options.size();i++)
		if ( m_Options[i].sName == pName )
			return &m_Options[i];
	return NULL;
}

inline int Property::size() const
{
   return m_pType->size();
}

inline bool Property::isPointer() const
{
   return m_pType->isPointer();
}

inline bool Property::isReference() const
{
   return m_pType->isReference();
}

inline int Property::count( void * pThis ) const
{
   return m_pContainer->count( containerPointer( pThis ) );
}

inline void * Property::containerPointer( void * pThis ) const
{
   return (byte *)pThis + offset();
}

inline void * Property::pointer( void * pThis, int n ) const
{
   return m_pContainer->pointer( containerPointer( pThis ), n );
}

inline bool Property::isDynamic() const
{
	return m_pContainer->isDynamic();
}

inline void Property::allocate( void * pThis, int nSize )
{
	m_pContainer->allocate( containerPointer( pThis ), nSize );
}

inline int Property::add( void * pThis )
{
   return m_pContainer->add( containerPointer( pThis ) );
}

inline bool Property::remove( void * pThis, int n )
{
   return m_pContainer->remove( containerPointer( pThis ), n );
}

inline void Property::clear( void * pThis )
{
   m_pContainer->clear( containerPointer( pThis ) );
}


//----------------------------------------------------------------------------

inline Property::Option & Property::Option::operator=( const Option & copy )
{
	sName = copy.sName;
	sValue = copy.sValue;
	return *this;
}

inline Property::Find & Property::Find::operator=( const Find & copy )
{
	pThis = copy.pThis;
	pProperty = copy.pProperty;
	nElement = copy.nElement;
	pData = copy.pData;
	pType = copy.pType;
	sExpr = copy.sExpr;
	return *this;
}

//---------------------------------------------------------------------------------------------------

inline void Property::Container::allocate( void * pThis, int nSize )
{}

inline int Property::Container::add( void * pThis )
{
   return -1;
}

inline bool Property::Container::remove( void * pThis, int n )
{
   return false;
}

inline void Property::Container::clear( void * pThis )
{}

//---------------------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
