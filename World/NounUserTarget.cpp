/**
	@file NounUserTarget.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 7/25/2010 8:38:32 PM
*/

#include "NounUserTarget.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_FACTORY( NounUserTarget, NounTarget );

BEGIN_PROPERTY_LIST( NounUserTarget, NounTarget )
	ADD_TRANSMIT_UPDATE_PROPERTY( m_nUserId );
END_PROPERTY_LIST();

NounUserTarget::NounUserTarget()
{}

NounUserTarget::NounUserTarget( dword nUserId ) 
{
	m_nUserId = nUserId;
}

//---------------------------------------------------------------------------------------------------
//EOF
