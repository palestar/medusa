/*
	Property.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "PropertyList.h"

#include "Standard/StringHash.h"

#pragma warning( disable : 4996 )	// warning C4996: 'sscanf': This function or variable may be unsafe. Consider using sscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

//----------------------------------------------------------------------------

Property::Property( Container * pContainer, const char * pName, int nOffset, Type * pType, 
						 dword nFlags /*= 0*/, const char * pNote /*= NULL*/ )
	: m_pContainer( pContainer ), 
	m_nId( 0 ), 
	m_pName( pName ), 
	m_nNameId( StringHash( pName ) ), 
	m_nOffset( nOffset ), 
	m_pType( pType ), 
	m_nFlags( nFlags ), 
	m_pNote( pNote != NULL ? pNote : pName )
{}

Property::Property( const Property & copy ) 
	: m_pContainer( copy.m_pContainer ), 
	m_nId( copy.m_nId ), 
	m_pName( copy.m_pName ),
	m_nNameId( copy.m_nNameId ),
	m_nOffset( copy.m_nOffset ), 
	m_pType( copy.m_pType ), 
	m_nFlags( copy.m_nFlags ),
	m_pNote( copy.m_pNote ), 
	m_Options( copy.m_Options )
{}

Property::~Property()
{}

//----------------------------------------------------------------------------

void Property::setPropertyId( dword nId )
{
	m_nId = nId;
}

void Property::setFlags( dword nFlags )
{
	m_nFlags = nFlags;
}

void Property::addFlags( dword nFlags )
{
	m_nFlags |= nFlags;
}

void Property::clearFlags( dword nFlags )
{
	m_nFlags &= ~nFlags;
}

void Property::setNote( const char * pNote )
{
	m_pNote = pNote;
}

void Property::addOption( const CharString & name, const CharString & value )
{
	Option option;
	option.sName = name;
	option.sValue = value;
	m_Options.push( option );
}

void Property::removeOption( int n )
{
	if ( m_Options.isValid( n ) )
		m_Options.remove( n );
}

//----------------------------------------------------------------------------

bool Property::enumerate( void * pThis, Array< Find > & found )
{
	if ( pThis == NULL )
		return true;

	int nCount = count( pThis );
	for(int i=0;i<nCount;i++)
	{
		CharString sBase;
		if ( nCount > 1 )
			sBase.format("%s[%d]", m_pName, i );
		else
			sBase = m_pName;
		
		Find f;
		f.pProperty = this;
		f.nElement = i;
		f.pData = pointer( pThis, i );
		f.pType = m_pType;
		f.sExpr = sBase;
		found.push( f );

		if ( m_pType->propertyList() != NULL )
		{
			Array< Find > subse;
			if ( isPointer() )
			{
				sBase += "->";		// pointer accessor
				void ** pPointer = (void **)f.pData;
				if (! m_pType->propertyList()->enumerate( *pPointer, subse ) )
					return false;		// failed to enumerate properties
			}
			else
			{
				sBase += ".";		// member accessor
				if (! m_pType->propertyList()->enumerate( f.pData, subse ) )
					return false;		// failed to enumerate properties
			}

			for(int j=0;j<(int)subse.size();j++)
			{
				Find f;
				f.pProperty = subse[j].pProperty;
				f.nElement = subse[j].nElement;
				f.pData = subse[j].pData;
				f.pType = subse[j].pType;
				f.sExpr = sBase + subse[j].sExpr;
				found.push( f );
			}
		}
	}

	return true;
}

bool Property::find( const Expression & e, void * pThis, Find & found )
{
	if ( pThis == NULL )
		return false;
	if ( e.type() != Expression::TOKEN_SYMBOL || stricmp<char>( e.string(), m_pName ) != 0 )
		return false;	

	Expression p( e );
	p.next();

	int nElement = 0;
	if ( p.type() == Expression::TOKEN_ELEMENT )
	{
		if( sscanf( p.string(), "%d", &nElement ) != 1 )
			return false;
		if (! isDynamic() )
		{
			int nCount = count( pThis );
		if ( nElement < 0 || nElement >= nCount )
			return false;		// invalid array element!
		}
		p.next();
	}

	if ( p.type() == Expression::TOKEN_POINTER )
	{
		if ( m_pType->propertyList() == NULL )
			return false;     // unknown type

			p.next();
			if ( p.type() != Expression::TOKEN_SYMBOL )
				return false;		// no symbol found after the ->
			if ( !isPointer() )
				return false;		// can't use a -> on a non-pointer type


			void ** pPointer = (void **)pointer( pThis, nElement );
			return m_pType->propertyList()->find( p, *pPointer, found );
	}
	else if ( p.type() == Expression::TOKEN_MEMBER )
	{
		if ( m_pType->propertyList() == NULL )
			return false;     // unknown type
			p.next();
		if ( p.type() != Expression::TOKEN_SYMBOL )
			return false;		// no symbol found after the .

		return m_pType->propertyList()->find( p, pointer( pThis, nElement ), found );
	}
	
	// property resolved, return results...
	found.pThis = pThis;
	found.pProperty = this;
	found.nElement = nElement;
	found.pData = pointer( pThis, nElement );
	found.pType = m_pType;
	return true;
}

//----------------------------------------------------------------------------
//EOF
