/**
	@file DisplayEffectHDR.h
	@brief HDR with bloom effect..

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/5/2010 5:10:22 PM
*/

#ifndef DISPLAYEFFECTHDR_H
#define DISPLAYEFFECTHDR_H

#include "Display/DisplayEffect.h"
#include "Shader.h"

#include <d3dx9.h>

//---------------------------------------------------------------------------------------------------

class DisplayEffectHDR : public DisplayEffect
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	//! Construction
	DisplayEffectHDR();
	virtual ~DisplayEffectHDR();

	//! DisplayEffect interface
	virtual bool				preRender( DisplayDevice * pDevice );		
	virtual bool				postRender( DisplayDevice * pDevice );		
	virtual void				release();

	//! Accessors
	int							bloomLevels() const;				// how many steps for the bloom process
	//! Mutators
	void						setBloomLevels( int nBloomLevels );

protected:
	//! Data
	int							m_nBloomLevels;					// how much bloom to apply
	int							m_nBloomSize;					// divide the frame buffer size by this amount

	SizeInt						m_RenderSize;
	SizeInt						m_BloomSize;
	IDirect3DTexture9 *			m_pBloomRenderTarget;			// the texture our scene is actually rendered into..
	IDirect3DTexture9 *			m_pBloomTextures[ 2 ];			// bloom is rendered back and forth between these two buffers to produce final image
	IDirect3DSurface9 *			m_pBloomSurfaces[ 2 ];
	IDirect3DSurface9 *			m_pRenderTarget;				// this is our final render target
	IDirect3DStateBlock9 *		m_pStateBlock;					// state block for restoring the rendering

	Shader::Ref					m_pBrightPass;
	Shader::Ref					m_pHorzBlur;
	Shader::Ref					m_pVertBlur;
	Shader::Ref					m_pCombine;

	bool						m_bBloomInitialized;
	bool						m_bBloomFailed;
};

//---------------------------------------------------------------------------------------------------

inline int DisplayEffectHDR::bloomLevels() const
{
	return m_nBloomLevels;
}

inline void DisplayEffectHDR::setBloomLevels( int nBloomLevels )
{
	m_nBloomLevels = nBloomLevels;
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF
