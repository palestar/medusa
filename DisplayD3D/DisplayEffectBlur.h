/**
	@file DisplayEffectBlur.h
	@brief This display effect is used to blur the display, this effect works by rendering the current
	frame and the previous frame (slightly darker) into the destination.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/21/2010 9:31:07 PM
*/

#ifndef DISPLAYEFFECTBLUR_H
#define DISPLAYEFFECTBLUR_H

#include "Display/DisplayEffect.h"
#include "Shader.h"

#include <d3dx9.h>

//---------------------------------------------------------------------------------------------------

class DisplayEffectBlur : public DisplayEffect
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	//! Construction
	DisplayEffectBlur();
	virtual ~DisplayEffectBlur();

	//! DisplayEffect interface
	virtual bool				preRender( DisplayDevice * pDevice );		
	virtual bool				postRender( DisplayDevice * pDevice );		
	virtual void				release();

	//! Accessors
	float						blur() const;					// each update the previous frame is darkened by this amount
																// 0.0f = inifinite blur, no loss from previous frame
																// 1.0f = no blur
	//! Mutators
	void						setBlur( float fBlur );		

protected:
	//! Data
	float						m_fBlur;						// the amount to darken our previous frame
	SizeInt						m_BufferSize;
	IDirect3DTexture9 *			m_pBlurRenderTarget;			// the texture our scene is actually rendered into..
	IDirect3DTexture9 *			m_pPreviousTexture;				// texture of our previous frame
	IDirect3DSurface9 *			m_pPreviousSurface;		
	IDirect3DSurface9 *			m_pRenderTarget;				// this is our final render target
	IDirect3DStateBlock9 *		m_pStateBlock;					// state block for restoring the rendering
	Shader::Ref					m_pCombine;						// shader to render our current frame combined with our previous frame
	Shader::Ref					m_pScale;

	bool						m_bBlurInitialized;
	bool						m_bBlurFailed;
};

//---------------------------------------------------------------------------------------------------

inline float DisplayEffectBlur::blur() const
{
	return m_fBlur;
}

inline void DisplayEffectBlur::setBlur( float fBlur )
{
	m_fBlur = fBlur;
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF
