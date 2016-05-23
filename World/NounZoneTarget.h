/*
	NounZoneTarget.h
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef NOUNZONETARGET_H
#define NOUNZONETARGET_H

#include "NounTarget.h"
#include "NodeZone.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

class DLL NounZoneTarget : public NounTarget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< NounZoneTarget >			Ref;
	typedef TargetReference< NounZoneTarget >	tRef;

	// Construction
	NounZoneTarget();
	NounZoneTarget( NodeZone * pZone, const Vector3 & vPosition );
	NounZoneTarget( const Vector3 & vWorldPosition, WorldContext * pContext );

	// Widget interface
	bool				write( const OutStream & output );
	bool				read( const InStream & input );
	// Noun interface
	virtual void		calculateWorld() const;			
	// Mutators
	void				setPosition( const Vector3 & vWorldPosition, WorldContext * pContext );
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

