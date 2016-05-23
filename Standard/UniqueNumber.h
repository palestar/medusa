/*
	UniqueNumber.h
	64-bit GUID class

	The UniqueNumber class provides unique ID's for objects for serialization and communications purposes. 
	UniqueNumbers are created on both the client & server, and to avoid conflicts between the two systems 
	the upper 2 bits of all ID's are used to specify the type of ID.

	0x00 = Server 
	0x01 = Client
	0x02 = <Reserved>
	0x03 = <Reserved>

	[--]								Type (2 Bits)
	[------------------------------]	PersistID (30 Bits)
	[--------------------------------]	AtomicID (32 Bits)

	(c)2005 Palestar, Richard Lyle
*/

#ifndef UNIQUE_NUMBER_H
#define UNIQUE_NUMBER_H

#include "Exception.h"
#include "String.h"
#include "Types.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

enum UniqueNumberType {
	UNT_SERVER		= 0x0,
	UNT_CLIENT		= 0x1,
	UNT_RESERVED1	= 0x2,
	UNT_RESERVED2	= 0x3,
};

const qword TYPE_BITS		= 2;
const qword TYPE_SHIFT		= 62;
const qword TYPE_MASK		= (((qword)1 << TYPE_BITS)-1) << TYPE_SHIFT;

const qword PERSISTID_BITS	= 30;
const qword PERSISTID_SHIFT	= 32;
const qword PERSISTID_MASK	= (((qword)1 << PERSISTID_BITS)-1) << PERSISTID_SHIFT;

const qword ATOMICID_BITS	= 32;
const qword ATOMICID_SHIFT  = 0;
const qword ATOMICID_MASK	= (((qword)1 << ATOMICID_BITS)-1) << ATOMICID_SHIFT;

//---------------------------------------------------------------------------------------------------

//! Exceptions
class UniqueNumberExhaused : public Exception<UniqueNumberExhaused>
{};
class UniqueNumberInvalidPersistID : public Exception<UniqueNumberInvalidPersistID>
{};

//---------------------------------------------------------------------------------------------------

//! This abstract interface is implemented by different application types to provide unique ID's
//! to the UniqueNumber class.
class DLL UniqueNumberProvider 
{
public:
	virtual qword		requestID() = 0;
};

//---------------------------------------------------------------------------------------------------

//! This provider is for client use only, it provides unique numbers only for the local client and is NOT
//! guaranteed to conflict with conflict with other client's and servers.
class DLL PersistedUniqueNumberProvider : public UniqueNumberProvider
{
public:
	PersistedUniqueNumberProvider( UniqueNumberType a_eType, dword a_nPersistID, const char * a_pPersistFile = NULL );
	
	//! UniqueNumberProvider interface
	virtual qword		requestID();

protected:
	//! Data
	UniqueNumberType	m_eType;
	dword				m_nPersistID;			// upper 30-bits of all ID's
	dword				m_nWrapCount;			// number of times the lower 32-bits have wrapped
	dword				m_nAtomicID;			// lower 32-bits of all ID's
	qword				m_nPersistTypeMask;		// mask of m_eType & m_nPersistID

	bool				m_bEnablePersistFile;	// true if we are persisting between runs using a file
	CharString			m_sPersistFile;			// name of that file
	dword				m_nLastAtomicID;		// last ID of the current block of atomic ID written to the persist file
};

//---------------------------------------------------------------------------------------------------

//! This provided uses the current time in the upper 32-bits and XOR's with the current CPU tick. It would 
//! be VERY hard to generate the same unique ID using this provider, but it's not guaranteed.
class DLL TimeUniqueNumberProvider : public UniqueNumberProvider
{
public:
	TimeUniqueNumberProvider();
	//! UniqueNumberProvider interface
	virtual qword requestID();
protected:
	//! Data
	qword			m_nSeedID;
	qword			m_nMachineID;
	dword			m_nAtomicID;
};

//---------------------------------------------------------------------------------------------------

// This class, automatically creates a UniqueNumber when the default constructor is called
class DLL UniqueNumber
{
public:
    // Construction
    UniqueNumber( bool a_bGenerateKey = true );		// automatically create a new unique id
	UniqueNumber( const UniqueNumber & );
	UniqueNumber( const char * );		
    UniqueNumber( qword nid );

    // Accessors
	operator		qword() const;
    CharString		string() const;						// convert number to string
	CharString		hexString() const;

    bool            operator==(const UniqueNumber &compare) const;
    bool            operator!=(const UniqueNumber &compare) const;
    bool            operator> (const UniqueNumber &compare) const;
    bool            operator>=(const UniqueNumber &compare) const;
    bool            operator< (const UniqueNumber &compare) const;
    bool            operator<=(const UniqueNumber &compare) const;

	// Mutators
	void			set( const char * pKey );
	void			setHex( const char * pHexKey );

	//! Generate a new UniqueNumber..
	void			requestNumber();

	UniqueNumber & operator+=( const UniqueNumber & add )
	{
		m_Id += add.m_Id;
		return *this;
	}

	UniqueNumber &	operator=( const UniqueNumber & assign )
	{
		m_Id = assign.m_Id;
		return *this;
	}

	//! This function is used to set the provider, if NULL is provided then the default provider
	//! is used.
	static bool		setProvider( UniqueNumberProvider * pProvider = NULL );

	//! Data
	qword			m_Id;

protected:
	//! Static Data
	static UniqueNumberProvider *
					sm_pProvider;
};

//----------------------------------------------------------------------------

inline UniqueNumber::operator qword() const
{ 
	return m_Id; 
}

inline bool UniqueNumber::operator==(const UniqueNumber &compare) const   
{ 
	return m_Id == compare.m_Id;
}

inline bool UniqueNumber::operator!=(const UniqueNumber &compare) const   
{ 
	return m_Id != compare.m_Id; 
}

inline bool UniqueNumber::operator> (const UniqueNumber &compare) const   
{ 
	return m_Id >  compare.m_Id; 
}

inline bool UniqueNumber::operator>=(const UniqueNumber &compare) const   
{ 
	return m_Id >= compare.m_Id; 
}

inline bool UniqueNumber::operator< (const UniqueNumber &compare) const   
{ 
	return m_Id <  compare.m_Id; 
}

inline bool UniqueNumber::operator<=(const UniqueNumber &compare) const   
{ 
	return m_Id <= compare.m_Id;
}

#endif

//----------------------------------------------------------------------------
// EOF
