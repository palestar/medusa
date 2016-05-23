/**
	@file Atomic.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 8/8/2010 2:53:23 PM
*/

#include "Atomic.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//---------------------------------------------------------------------------------------------------

int Atomic::increment( volatile int * a_pInteger )
{
#if defined(_WIN32)
	return InterlockedIncrement( (LONG *)a_pInteger );
#else
	return __sync_add_and_fetch( a_pInteger, 1 );
#endif
}

int Atomic::add( volatile int * a_pInteger, int a_nAdd )
{
#if defined(_WIN32)
	return InterlockedExchangeAdd( (LONG *)a_pInteger, (LONG)a_nAdd );
#else
	return __sync_add_and_fetch( a_pInteger, a_nAdd );
#endif

}

int Atomic::decrement( volatile int * a_pInteger )
{
#if defined(_WIN32)
	return InterlockedDecrement( (LONG *)a_pInteger );
#else
	return __sync_sub_and_fetch( a_pInteger, 1 );
#endif
}

int Atomic::swap( volatile int * a_pInteger, int a_nValue )
{
#if defined(_WIN32)
	return InterlockedExchange( (LONG *)a_pInteger, (LONG)a_nValue );
#else
	return __sync_lock_test_and_set( a_pInteger, a_nValue );
#endif
}

bool Atomic::compareSwap( volatile int * a_pInteger, int a_nCompare, int a_nValue )
{
#if defined(_WIN32)
	return InterlockedCompareExchange( (LONG *)a_pInteger, (LONG)a_nValue, (LONG)a_nCompare ) == a_nCompare;
#else
	return __sync_val_compare_and_swap( a_pInteger, a_nCompare, a_nValue ) == a_nCompare;
#endif
}

//---------------------------------------------------------------------------------------------------
//EOF
