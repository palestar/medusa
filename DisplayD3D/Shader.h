/**
	@file Shader.h
	@brief TODO

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/9/2010 8:01:52 AM
*/

#ifndef SHADER_H
#define SHADER_H

#include "Factory/Widget.h"
#include "Standard/WeakReference.h"

#include <map>									// STL
#include <string>
#include <d3d9.h>								// DirectX 9.0
#include <d3dx9.h>

class DisplayDeviceD3D;		// forward declare

//---------------------------------------------------------------------------------------------------

class Shader : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	typedef Reference< Shader >		Ref;
	typedef WeakReference< Shader >	WeakRef;

	//! Construction
	Shader();
	virtual ~Shader();

	//! Accessors
	bool						valid() const;					// true if this shader is ready to be used.
	bool						released() const;				// true if this shader has been released
	const CharString &			shaderName() const;				// filename of this shader

	IDirect3DVertexShader9 *	vertexShader() const;		
	ID3DXConstantTable *		vertexShaderConstants() const;
	IDirect3DPixelShader9 *		pixelShader() const;
	ID3DXConstantTable *		pixelShaderConstants() const;

	//! Mutators
	bool						load( DisplayDeviceD3D * pDevice, const char * pShaderName );
	void						release();
	
	void						setConstant( const char * pConstant, const Value & value );
	void						clearConstant( const char * pConstant );

	bool						apply( DisplayDeviceD3D * pDevice );
	void						applyConstants( DisplayDeviceD3D * pDevice, 
									ID3DXConstantTable * pTable );

	static bool					applyConstant( DisplayDeviceD3D * pDevice,
									ID3DXConstantTable * pTable, 
									const char * pName, const Value & value );

protected:
	//! Types
	typedef std::map< std::string, Value >		ConstantMap;

	//! Data
	bool						m_bValid;					// true if this shader is ready to be used.
	bool						m_bReleased;				// true if this shader has been released
	CharString					m_sShaderName;				// filename of this shader
	dword						m_nShaderFileTime;			// timestamp from the shader file

	IDirect3DVertexShader9 *	m_pVS;		
	ID3DXConstantTable *		m_pVSConstants;
	IDirect3DPixelShader9 *		m_pPS;		
	ID3DXConstantTable *		m_pPSConstants;

	ConstantMap					m_Constants;

};

//---------------------------------------------------------------------------------------------------

inline bool Shader::valid() const
{
	return m_bValid;
}

inline bool Shader::released() const
{
	return m_bReleased;
}

inline const CharString & Shader::shaderName() const
{
	return m_sShaderName;
}

inline IDirect3DVertexShader9 * Shader::vertexShader() const
{
	return m_pVS;
}

inline ID3DXConstantTable * Shader::vertexShaderConstants() const
{
	return m_pVSConstants;
}

inline IDirect3DPixelShader9 * Shader::pixelShader() const
{
	return m_pPS;
}

inline ID3DXConstantTable * Shader::pixelShaderConstants() const
{
	return m_pPSConstants;
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF
