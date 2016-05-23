/**
	@file DisplayEffect.h
	@brief This is the base class for any rendering effects (e.g. Bloom, Blur, Depth of field, etc..)

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/5/2010 2:50:46 PM
*/

#ifndef DISPLAYEFFECT_H
#define DISPLAYEFFECT_H

#include "Factory/FactoryTypes.h"
#include "Standard/WeakReference.h"
#include "MedusaDll.h"

class DisplayDevice;		// forward declare

//---------------------------------------------------------------------------------------------------

class DLL DisplayEffect : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	//! Types
	typedef Reference< DisplayEffect >		Ref;
	typedef WeakReference< DisplayEffect >	WeakRef;

	// Construction
	DisplayEffect() 
	{}
	virtual ~DisplayEffect()
	{}

	//! Interface
	virtual bool				preRender( DisplayDevice * pDevice ) = 0;		// This is invoked before we render our scene
	virtual bool				postRender( DisplayDevice * pDevice ) = 0;		// This is invoked after the scene and all passes have been rendered.
	virtual void				release() = 0;									// when called the effect should release all allocated resources
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
