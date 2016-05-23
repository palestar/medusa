/*
	Singleton.h

	A singleton template class
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef SINGLETON_H
#define SINGLETON_H

//----------------------------------------------------------------------------

template<typename T>
class Singleton
{
public:
	// Static
	static T * instance()
	{
		if ( s_pInstance == NULL )
			s_pInstance = new T;
		return s_pInstance;
	}

	static void release()
	{
		delete s_pInstance;
		s_pInstance = NULL;
	}

protected:
	// protect the constructor and destuctor
	Singleton()
	{}
	virtual ~Singleton()
	{}

private:
	// can not copy a singleton
	Singleton( const Singleton & copy )
	{}

	// Data
	static T * s_pInstance;
};

template<typename T>
T * Singleton<T>::s_pInstance = NULL;

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
