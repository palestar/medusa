/*
	Value.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/

#include "Reflection/Value.h"

//---------------------------------------------------------------------------------------------------

Value::Value() : m_pType( NULL ), m_pData( NULL )
{}

Value::Value( const Value & copy ) : m_pType( NULL ), m_pData( NULL )
{
	set( copy.type(), copy.data() );
}

Value::Value( Type * pType, const void * pData ) : m_pType( NULL ), m_pData( NULL )
{
	set( pType, pData );
}

Value::~Value() 
{
	release();
}

Type * Value::type() const
{
	return m_pType;
}

void * Value::data() const
{
	return m_pData;
}

bool Value::set( Type * pType, const void * pData )
{
	bool bSuccess = true;

	release();

	if ( pType != NULL )
	{
		m_pType = pType;
		m_pData = m_pType->allocate();
		// copy the value into this object
		if ( pData != NULL )
			bSuccess = Type::copy( m_pData, m_pType, pData, m_pType );
	}

	return bSuccess;
}

void Value::setNoCopy( Type * pType, void * pData )
{
	release();
	m_pType = pType;
	m_pData = pData;
}

bool Value::copy( Type * pType, const void * pData )
{
	// if the type is the SAME and we already have allocate the data, copy into our existing data..
	if ( m_pType == pType && m_pData != NULL )
	{
		if ( !Type::copy( m_pData, m_pType, pData, pType ) )
			return false;
	}

	return set( pType, pData );
}

void Value::release()
{
	if ( m_pType.valid() && m_pData != NULL  )
		m_pType->free( m_pData );

	m_pType = NULL;
	m_pData = NULL;
}

bool Value::read( const InStream & input )
{
	release();

	dword nTypeNameHash;
	input >> nTypeNameHash;

	if ( nTypeNameHash != 0L )
	{
		m_pType = Type::findNameHash( nTypeNameHash );
		if (! m_pType )
			return false;
		m_pData = m_pType->allocate();
		if (! m_pData )
			return false;

		return m_pType->read( input, m_pData );
	}

	return true;
}

bool Value::write( const OutStream & output ) const
{
	if ( m_pType && m_pData )
	{
		output << m_pType->nameHash();
		return m_pType->write( output, m_pData );
	}

	output << 0L;
	return true;
}

//---------------------------------------------------------------------------------------------------
//EOF
