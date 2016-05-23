/**
	@file NounUserTarget.h
	@brief Version of NounTarget which will serialize a user ID.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 7/25/2010 8:37:10 PM
*/

#ifndef NOUNUSERTARGET_H
#define NOUNUSERTARGET_H

#include "NounTarget.h"
#include "WorldDll.h"

//---------------------------------------------------------------------------------------------------

class DLL NounUserTarget : public NounTarget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	//! Construction
	NounUserTarget();
	NounUserTarget( dword nUserId );
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
