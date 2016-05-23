/**
	@file HashMap.h
	@brief TODO

	(c)2012 Palestar Inc
	@author Richard Lyle @date 4/8/2012 7:32:38 PM
*/

#ifndef HASHMAP_H
#define HASHMAP_H

//---------------------------------------------------------------------------------------------------

#ifdef __GNUC__

#include <ext/hash_map>

template<typename K,typename T>
class HashMap : public __gnu_cxx::hash_map<K,T>
{};

#else

#include <hash_map>
template<typename K,typename T>
class HashMap : public stdext::hash_map<K,T>
{};

#endif

#endif

//---------------------------------------------------------------------------------------------------
//EOF
