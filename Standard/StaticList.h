/*
	StaticList.h

	This mixin class is used to build a linked list of objects automatically. This class uses a semaphore
	so the list can be thread safe.

	Usage:

	class MyClass : public StaticList<MyClass>
	{
		....
	};


	// create several objects
	new MyClass;
	new MyClass;
	new MyClass;

	MyClass::staticListSempahore().lock();

	// now enumerate all the created objects
	List<MyClass *>::Iterator it = MyClass::staticList().head();
	while( it.valid() )
	{
		MyClass * pClass = *it;

		...

		it++;
	}

	MyClass::staticListSemaphore().unlock();

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef STATICLIST_H
#define STATICLIST_H

#include "Standard/List.h"
#include "Standard/CriticalSection.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

template<class T>
class StaticList
{
public:
	// Construction
	StaticList( bool bKeepLocked = false )
	{
		AutoLock lock( &staticListSemaphore() );
		m_Self = staticList().insert( (T *)this );
		if ( bKeepLocked )
			staticListSemaphore().lock();
	}
	virtual ~StaticList()
	{
		AutoLock lock( &staticListSemaphore() );
		staticList().remove( m_Self );
	}
	
	// Mutators
	static CriticalSection &	staticListSemaphore();
	static List<T *> &	staticList();

private:
	// Data
	typename List<T *>::Iterator		
							m_Self;

	// Static
	static CriticalSection	s_Lock;
	static List<T *>		s_List;
};

//----------------------------------------------------------------------------

template<class T>
CriticalSection StaticList<T>::s_Lock;

template<class T>
List<T *> StaticList<T>::s_List;

//----------------------------------------------------------------------------

template<class T>
inline CriticalSection & StaticList<T>::staticListSemaphore()
{
	return s_Lock;
}

template<class T>
inline List<T *> & StaticList<T>::staticList()
{
	return s_List;
}


//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
