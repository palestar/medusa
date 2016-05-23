/*
	PropertyClass.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "PropertyClass.h"

PropertyList * PropertyClass::propertyList()								               
{																				               
	return staticPropertyList();										               
}		

PropertyList * PropertyClass::staticPropertyList()						               
{																				               
   static PropertyListTemplate<PropertyClass> LIST( NULL );	
	if (! LIST.initialized() )							
		LIST.initialize();
   return &LIST;
}

//---------------------------------------------------------------------------------------------------

bool PropertyClass::copy( void * pLeft, Type * pLeftType, const void * pRight, Type * pRightType, Property * pProperty, int nElement, bool bMutator )
{
   return TypeCopy::copy( pLeft, pLeftType, pRight, pRightType );
}

void PropertyClass::OnPreSave()
{}

void PropertyClass::OnPostSave()
{}

void PropertyClass::OnPreLoad()
{}

void PropertyClass::OnPostLoad()
{}

//----------------------------------------------------------------------------
//EOF
