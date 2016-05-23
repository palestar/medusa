/*
	Resource.h

	Base class for all resources..
	(c)2005 Palestar, Richard Lyle
*/

#ifndef RESOURCE_H
#define RESOURCE_H

#include "Standard/Referenced.h"

#include "Factory/Widget.h"
#include "ResourceLink.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Resource : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference< Resource >			Ref;
	typedef ResourceLink< Resource >		Link;

	// Construction
	Resource();
};

#include "Factory/FactoryTypes.h"

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF
