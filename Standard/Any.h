/**
	@file Any.h
	@brief This class will hold any type of data that you would like to store into it. The Any object
	can then be passed through functions, stored in a class, etc. You must know the type of data stored
	however to get it out as the correct type using the get<> function. 

	If you need a more type-safe system, see the Value class.

	(c)2012 Palestar Inc
	@author Richard Lyle @date 12/27/2012 4:51:06 PM
*/

#ifndef ANY_H
#define ANY_H

//---------------------------------------------------------------------------------------------------

class Any
{
public:
	Any() : m_pData( 0 )
	{}
	Any( const Any & a_Copy ) : m_pData( a_Copy.m_pData != 0 ? a_Copy.m_pData->copy() : 0 )
	{}
	template<typename T>
	Any( T a_Data ) : m_pData( new DataStoreT<T>( a_Data ) )
	{}
	virtual ~Any()
	{
		if ( m_pData != 0 )
			delete m_pData;
	}

	bool valid() const
	{
		return m_pData != 0;
	}
	template<typename T>
	T & get()
	{
		return ((DataStoreT<T> *)m_pData)->m_Data;
	}
	template<typename T>
	void set(T a_Data )
	{
		if ( m_pData != 0 )
			delete m_pData;
		m_pData = new DataStoreT<T>( a_Data );
	}

protected:
	struct DataStore 
	{
		virtual ~DataStore()
		{}
		virtual DataStore * copy() = 0;
	};

	template<typename T>
	struct DataStoreT : public DataStore
	{
		DataStoreT( T a_Data ) : m_Data( a_Data )
		{}
		virtual DataStore * copy()
		{
			return new DataStoreT<T>( m_Data );
		}
		T 		m_Data;
	};
	DataStore *		m_pData;
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
