/**
	@file WeakDelegate.h
	@brief This class basically is a function pointer to a object member with a WeakReference.

	class MyClass : public Referenced
	{
	public:
		void MyFunction( int a )
		{
			...
		}
	};

	MyClass * pObject = new MyClass();
	WeakDelegate< int > d = WeakDelegate<int>::Create<MyClass,&MyClass::MyFunction>( pObject );

	// invoke the delegate
	d( 50 );
	// destroy the object
	delete pObject;
	// try to invoke the delegate, it will fail silently..
	d( 100 );


	(c)2009 Palestar Inc
	@author Richard Lyle @date 3/31/2010 8:51:59 PM
*/

#ifndef WEAKDELEGATE_H
#define WEAKDELEGATE_H

#include "Reference.h"
#include "WeakReference.h"

//---------------------------------------------------------------------------------------------------

template<typename T>
class WeakDelegate
{
public:
	//! Construction
	WeakDelegate() : m_pStub( NULL )
	{}
	WeakDelegate( const WeakDelegate & a_Copy ) 
	{
		*this = a_Copy;
	}
    ~WeakDelegate()
	{}

	WeakDelegate & operator=( const WeakDelegate<T> & a_rhs )
	{
		m_wpObject = a_rhs.m_wpObject;
		m_pStub = a_rhs.m_pStub;
		return *this;
	}

	template <typename C,void (C::*METHOD)( T )>
	static WeakDelegate Create ( C * a_spObject )
	{
		WeakDelegate wd;
		wd.m_wpObject  = a_spObject;
		wd.m_pStub = &Stub<C, METHOD>;
		return wd;
	}

	//! Invoked this delegate with the given parameter
	bool operator() ( T a_parm ) const
	{
	   return (*m_pStub )( m_wpObject, a_parm  );
	}

	//! Returns true if this delegate is valid
	bool IsValid() const
	{
		return m_wpObject.IsValid();
	}

private:
	//! Types
	typedef bool ( * StubType ) ( Referenced *, T );

	//! Data
	WeakReference<Referenced>	
						m_wpObject;					// pointer to the object
	StubType			m_pStub;					// function pointer to our stub function

	template <typename C,void (C::*METHOD)(T a_parm)>
	static bool Stub( Referenced * a_pObject, T a_parm )
	{
		C * pObject = (C *)a_pObject;
		if ( pObject )
		{
			(pObject->*METHOD)( a_parm );
			return true;
		}

		return false;
	}
};


#endif

//---------------------------------------------------------------------------------------------------
//EOF
