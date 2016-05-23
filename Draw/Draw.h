/*
	Draw.h

	Used to draw into PrimitiveSurface
	(c)2005 Palestar, Richard Lyle
*/

#ifndef DRAW_H
#define DRAW_H

#include "Display/PrimitiveSurface.h"
#include "Standard/Reference.h"
#include "Draw/Image.h"
#include "Draw/Font.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL Draw : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference<Draw>		Ref;

	// Construction
	Draw();

	// Mutators
	virtual void			setSurface( PrimitiveSurface::Ref surface );

	virtual bool			draw( const PointInt &point, Image * pImage, int frame = 0);
	virtual bool			drawAlpha( const PointInt &point, Image * pImage, int frame = 0);
	virtual bool			draw( PointInt &point, Font * pFont, int ch, const Color &color );
	virtual bool			draw( PointInt &point, Font * pFont, const String &string, const Color &color );
	virtual bool			line( const PointInt &from, const PointInt &to, const Color &color );
	virtual bool			circle( const PointInt &center, int radius, const Color &color );

	virtual bool			setAlpha( Color colorKey );
	virtual bool			outline( Color outline, Color with );

	// Static
	static Ref				create( PrimitiveSurface * pSurface );	// creates the best draw object for this type of surface

protected:
	PrimitiveSurface::Ref	m_Surface;			// our surface
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF
