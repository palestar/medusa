/**
	@file Shader.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/9/2010 8:07:37 AM
*/

#define MEDUSA_TRACE_ON

#include "Shader.h"
#include "DisplayDeviceD3D.h"
#include "PrimitiveSetTransformD3D.h"
#include "PrimitiveMaterialD3D.h"
#include "File/FileDisk.h"


//---------------------------------------------------------------------------------------------------

IMPLEMENT_LIGHT_FACTORY( Shader, Widget );

struct ShaderVersion
{
	float				fVersion;		
	const char *		pVersionName;
	const char *		pEntryPoint;
};

ShaderVersion VERTEX_SHADER_VERSIONS[] =
{
	{ 3.0f,		"vs_3_0",		"vs_main30"		},
	{ 2.0f,		"vs_2_0",		"vs_main20"		},
	{ 2.0f,		"vs_2_0",		"vs_main"		}
};

ShaderVersion PIXEL_SHADER_VERSIONS[] =
{
	{ 3.0f,		"ps_3_0",		"ps_main30"		},
	{ 2.0f,		"ps_2_0",		"ps_main20"		},
	{ 2.0f,		"ps_2_0",		"ps_main"		}
};

//---------------------------------------------------------------------------------------------------

Shader::Shader() : m_bValid( false ),
	m_bReleased( false ),
	m_nShaderFileTime( 0 ),
	m_pVSConstants( NULL ),
	m_pVS( NULL ),
	m_pPSConstants( NULL ),
	m_pPS( NULL )
{}

Shader::~Shader() 
{
	release();
}

bool Shader::load( DisplayDeviceD3D * pDevice, const char * pShaderName )
{
	release();

	m_sShaderName = pShaderName;
	m_bReleased = false;

	m_nShaderFileTime = FileDisk::fileDate( DisplayDevice::sm_sShadersPath + m_sShaderName );
	if ( m_nShaderFileTime == 0 )
	{
		TRACE( CharString().format( "Failed to load shader %s, file not found!", m_sShaderName ) );
		return false;
	}

	// try to load vertex shader first
#if ENABLE_SHADER_DEBUGGING
	DWORD dwShaderFlags = D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION;
#else
#if defined(_DEBUG)
	DWORD dwShaderFlags = D3DXSHADER_DEBUG/*|D3DXSHADER_SKIPOPTIMIZATION*/;
#else
	DWORD dwShaderFlags = 0;
#endif
#endif
	float fVSVersion = 0.0f;
	for(int i=0;i<sizeof(VERTEX_SHADER_VERSIONS)/sizeof(VERTEX_SHADER_VERSIONS[0]);++i)
	{
		ShaderVersion & version = VERTEX_SHADER_VERSIONS[i];
		if ( pDevice->m_fVertexShaderVersion < version.fVersion )
			continue;		// skip, not supported on this hardware..

		LPD3DXBUFFER pVSErrors = NULL;
		LPD3DXBUFFER pProgram = NULL;
		if ( D3DXCompileShaderFromFile( DisplayDevice::sm_sShadersPath + m_sShaderName, NULL, NULL, 
			version.pEntryPoint, version.pVersionName, dwShaderFlags, &pProgram, &pVSErrors, &m_pVSConstants ) == D3D_OK )
		{
			pDevice->m_pD3DD->CreateVertexShader( (DWORD *)pProgram->GetBufferPointer(), &m_pVS );
			RELEASEDD( pProgram );

			fVSVersion = version.fVersion;
			break;
		}

		// failed to compile at this pixel shader level..
		if ( pVSErrors != NULL )
		{
			const char * pError = (const char *)pVSErrors->GetBufferPointer();
			if ( strstr( pError, "entrypoint not found" ) == NULL )	// ignore entry point not found errors!
				TRACE( CharString().format( "%s VS ERRORS: %s", pShaderName, pError ) );
		}

		RELEASEDD( pVSErrors );
		RELEASEDD( pProgram );
	}

	for(int i=0;i<sizeof(PIXEL_SHADER_VERSIONS)/sizeof(PIXEL_SHADER_VERSIONS[0]);++i)
	{
		ShaderVersion & version = PIXEL_SHADER_VERSIONS[i];
		if ( (fVSVersion > 0.0f && version.fVersion != fVSVersion) || pDevice->m_fPixelShaderVersion < version.fVersion )
			continue;		// skip, not supported on this hardware..

		// then pixel shader from same file..
		LPD3DXBUFFER pPSErrors = NULL;
		LPD3DXBUFFER pProgram = NULL;
		if ( D3DXCompileShaderFromFile( DisplayDevice::sm_sShadersPath + pShaderName, NULL, NULL, 
			version.pEntryPoint, version.pVersionName, dwShaderFlags, &pProgram, &pPSErrors, &m_pPSConstants ) == D3D_OK )
		{
			pDevice->m_pD3DD->CreatePixelShader( (DWORD *)pProgram->GetBufferPointer(), &m_pPS );
			RELEASEDD( pProgram );
			break;
		}

		if ( pPSErrors != NULL )
		{
			const char * pError = (const char *)pPSErrors->GetBufferPointer();
			if ( strstr( pError, "entrypoint not found" ) == NULL )	// ignore entry point not found errors!
				TRACE( CharString().format( "%s PS ERRORS: %s", pShaderName, pError ) );
		}

		RELEASEDD( pPSErrors );
		RELEASEDD( pProgram );
	}

	if ( m_pPS != NULL || m_pVS != NULL )
	{
		m_bValid = true;
		TRACE( CharString().format( "Shader '%s' loaded.", pShaderName ) );
	}
	else
	{
		m_bValid = false;
		TRACE( CharString().format( "ERROR: Failed to load shader '%s'.", pShaderName ) );
	}

	return m_bValid;
}

void Shader::release()
{
	RELEASEDD( m_pVSConstants );
	RELEASEDD( m_pVS );
	RELEASEDD( m_pPSConstants );
	RELEASEDD( m_pPS );

	m_bValid = false;
	m_bReleased = true;
	m_nShaderFileTime = 0;
}

void Shader::setConstant( const char * pConstant, const Value & value )
{
	m_Constants[ pConstant ] = value;
}

void Shader::clearConstant( const char * pConstant )
{
	m_Constants.erase( pConstant );
}

bool Shader::apply( DisplayDeviceD3D * pDevice )
{
	// we only reload shaders in debug mode.. 
	if ( DisplayDevice::sm_bEnableShaderDebug && m_nShaderFileTime != FileDisk::fileDate( DisplayDevice::sm_sShadersPath + m_sShaderName ) )
	{
		TRACE( CharString().format("Shader %s modified, reloading...", m_sShaderName) );
		load( pDevice, m_sShaderName );
	}

	if ( m_pVS != NULL )
	{
		if (! pDevice->setVertexShader( m_pVS, m_pVSConstants ) )
			return false;
		applyConstants( pDevice, m_pVSConstants );
	}
	else
	{
		pDevice->clearVertexShader();
	}

	if ( m_pPS != NULL )
	{
		if (! pDevice->setPixelShader( m_pPS, m_pPSConstants ) )
			return false;
		applyConstants( pDevice, m_pPSConstants );
	}
	else
	{
		pDevice->clearPixelShader();
	}

	return true;
}

//---------------------------------------------------------------------------------------------------

void Shader::applyConstants( DisplayDeviceD3D * pDevice, ID3DXConstantTable * pTable )
{
	// enumerate our constant map, set any constants found into our shader constant table..
	for( ConstantMap::iterator iConstant = m_Constants.begin();
		iConstant != m_Constants.end(); ++iConstant )
	{
		if (! applyConstant( pDevice, pTable, iConstant->first.c_str(), iConstant->second ) )
			TRACE( CharString().format( "Failed to apply constant %s to shader %s", iConstant->first.c_str(), m_sShaderName ) );
	}
}

bool Shader::applyConstant( DisplayDeviceD3D * pDevice, ID3DXConstantTable * pTable, const char * pName, const Value & value )
{
	static Type * TYPE_INT = TypeTemplate<int>::instance();
	static Type * TYPE_D3DXMATRIX = TypeTemplate<D3DXMATRIX>::instance();
	static Type * TYPE_D3DVECTOR = TypeTemplate<D3DVECTOR>::instance();
	static Type * TYPE_D3DXVECTOR4 = TypeTemplate<D3DXVECTOR4>::instance();
	static Type * TYPE_D3DCOLORVALUE = TypeTemplate<D3DCOLORVALUE>::instance();
	static Type * TYPE_FLOAT = TypeTemplate<float>::instance();
	static Type * TYPE_DOUBLE = TypeTemplate<double>::instance();
	static Type * TYPE_BOOL = TypeTemplate<bool>::instance();
	static Type * TYPE_SIZEINT = TypeTemplate<SizeInt>::instance();
	static Type * TYPE_SIZEFLT = TypeTemplate<SizeFloat>::instance();
	static Type * TYPE_VECTOR3 = TypeTemplate<Vector3>::instance();
	static Type * TYPE_MATRIX33 = TypeTemplate<Matrix33>::instance();
	static Type * TYPE_COLOR = TypeTemplate<Color>::instance();

	if (! pTable || !pName )
		return false;		// invalid table pointer!
	D3DXHANDLE hConstant = pTable->GetConstantByName( NULL, pName );
	if ( hConstant == NULL )
		return true;		// no constant by that name, not an error..

	bool bApplied = false;
	if ( value.type() == TYPE_INT )
		bApplied = pTable->SetInt( pDevice->m_pD3DD, hConstant, value ) == D3D_OK;
	else if ( value.type() == TYPE_D3DXMATRIX )
		bApplied = pTable->SetMatrix( pDevice->m_pD3DD, hConstant, (D3DXMATRIX *)value.data() ) == D3D_OK;
	else if ( value.type() == TYPE_D3DXVECTOR4 )
		bApplied = pTable->SetVector( pDevice->m_pD3DD, hConstant, (D3DXVECTOR4 *)value.data() ) == D3D_OK;
	else if ( value.type() == TYPE_FLOAT || value.type() == TYPE_DOUBLE )
        bApplied = pTable->SetFloat( pDevice->m_pD3DD, hConstant, value ) == D3D_OK;
	else if ( value.type() == TYPE_BOOL )
		bApplied = pTable->SetBool( pDevice->m_pD3DD, hConstant, (bool)value ) == D3D_OK;
	else if ( value.type() == TYPE_D3DCOLORVALUE )
		bApplied = pTable->SetFloatArray( pDevice->m_pD3DD, hConstant, (FLOAT *)value.data(), 4 ) == D3D_OK;
	else if ( value.type() == TYPE_D3DVECTOR )
	{
		D3DXVECTOR4 v( *((D3DVECTOR *)value.data()), 1.0f );
		bApplied = pTable->SetVector( pDevice->m_pD3DD, hConstant, &v ) == D3D_OK;
	}
	else if ( value.type() == TYPE_SIZEINT || value.type() == TYPE_SIZEFLT )
	{
		SizeFloat sz( value );
		bApplied = pTable->SetFloatArray( pDevice->m_pD3DD, hConstant, &sz.width, 2 ) == D3D_OK;
	}
	else if ( value.type() == TYPE_VECTOR3 )
	{
		Vector3 vValue( value );

		D3DXVECTOR4 v;
		v.x = vValue.x;
		v.y = vValue.y;
		v.z = vValue.z;
		v.w = 1.0f;

		bApplied = pTable->SetVector( pDevice->m_pD3DD, hConstant, &v ) == D3D_OK;
	}
	else if ( value.type() == TYPE_MATRIX33 )
	{
		Matrix33 mValue( value );

		D3DXMATRIX m;
		m._11 = mValue.i.x;
		m._12 = mValue.i.y;
		m._13 = mValue.i.z;
		m._14 = 0.0f;

		m._21 = mValue.j.x;
		m._22 = mValue.j.y;
		m._23 = mValue.j.z;
		m._24 = 0.0f;

		m._31 = mValue.k.x;
		m._32 = mValue.k.y;
		m._33 = mValue.k.z;
		m._34 = 0.0f;

		m._41 = 0.0f;
		m._42 = 0.0f;
		m._43 = 0.0f;
		m._44 = 1.0f;

		bApplied = pTable->SetMatrix( pDevice->m_pD3DD, hConstant, &m ) == D3D_OK;
	}
	else if ( value.type() == TYPE_COLOR )
	{
		D3DCOLORVALUE color = PrimitiveMaterialD3D::makeD3DCOLORVALUE( value );
		bApplied = pTable->SetFloatArray( pDevice->m_pD3DD, hConstant, (FLOAT *)&color, 4 ) == D3D_OK;
	}

	return bApplied;
}

//---------------------------------------------------------------------------------------------------
//EOF
