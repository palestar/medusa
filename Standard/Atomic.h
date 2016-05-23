/**
	@file Atomic.h
	@brief Atomic increment/decrement wrapper.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 8/8/2010 2:51:54 PM
*/

#ifndef ATOMIC_H
#define ATOMIC_H

#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class DLL Atomic 
{
public:
	//! Atomically increments the integer by it's pointer, returns the increment result
	static int		increment( volatile int * a_pInteger );
	//! Atomically adds a value to the integer, returns the original value before the add.
	static int		add( volatile int * a_pInteger, int a_nAdd );	
	//! Atomically decrement the integer, returns the result.
	static int		decrement( volatile int * a_pInteger );	
	//! Atomically swap the integer with the given value, returns the original value.
	static int		swap( volatile int * a_pInteger, int a_nValue );
	//! Atomically compare the integer to a_nCompare, if equal then it swaps with a_nValue and true on success, false on failure
	static bool		compareSwap( volatile int * a_pInteger, int a_nCompare, int a_nValue );
};

//---------------------------------------------------------------------------------------------------

// This is my attempt to implement a thread-safe atomic count that doesn't use locking. 
struct AtomicCount 
{
	AtomicCount( int a_nInitialCount = 0 ) : m_nCount( a_nInitialCount )
	{}

	// increment the count
	bool increment()
	{
		if ( m_nCount < 0 )
			return false;

		Atomic::increment( &m_nCount );
		return true;
	}

	// decrement the count, returns true when count hits 0, but only for one thread..
	bool decrement()
	{
		// if this was the last reference, destroy this object ..
		if ( Atomic::decrement( &m_nCount ) == 0 )
		{
			// try to destroy this object, try to replace the 0 in reference count with a -1, if we succeed
			// then destroy the object, otherwise stop since someone else grabbed a reference..
			if ( Atomic::compareSwap( &m_nCount, 0, -1 ) )
				return true;
		}

		return false;
	}

	volatile int	m_nCount;
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
