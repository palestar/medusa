/*
	Resource.cpp

	Resource base class
	(c)2005 Palestar, Richard Lyle
*/

#define RESOURCE_DLL
#include "Resource/Resource.h"
#include "Factory/FactoryTypes.h"

//-------------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( Resource, Widget );

Resource::Resource()
{}

//-------------------------------------------------------------------------------
// EOF
