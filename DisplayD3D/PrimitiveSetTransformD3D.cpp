/*
	PrimitiveSetTransformD3D.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "DisplayD3D/PrimitiveSetTransformD3D.h"
#include "DisplayD3D/PrimitiveFactory.h"

//------------------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveSetTransformD3D );

bool PrimitiveSetTransformD3D::execute()
{
	DisplayDeviceD3D * pDeviceD3D = (DisplayDeviceD3D *)m_pDevice;
	if ( pDeviceD3D->m_pD3DD->SetTransform( D3DTS_WORLD, &m_Matrix ) != D3D_OK)
		return false;
	
	// update vertex/pixel shader globals when the transform is changed
	if ( pDeviceD3D->m_pPSConstants != NULL )
		pDeviceD3D->updateGlobals( pDeviceD3D->m_pPSConstants );
	if ( pDeviceD3D->m_pVSConstants != NULL )
		pDeviceD3D->updateGlobals( pDeviceD3D->m_pVSConstants );
	
	return true;
}

void PrimitiveSetTransformD3D::clear()
{}

void PrimitiveSetTransformD3D::release()
{}

//----------------------------------------------------------------------------

const Matrix33 & PrimitiveSetTransformD3D::transform() const
{ 
	return(m_Transform); 
}

const Vector3 &	PrimitiveSetTransformD3D::transformOffset() const
{ 
	return(m_PostTransformOffset); 
}

void PrimitiveSetTransformD3D::setTransform( const Matrix33 &transform, const Vector3 &postTransformOffset )
{
	m_Transform = transform;
	m_PostTransformOffset = postTransformOffset;

	m_Matrix._11 = m_Transform.i.x;
	m_Matrix._12 = m_Transform.i.y;
	m_Matrix._13 = m_Transform.i.z;
	m_Matrix._14 = 0.0f;

	m_Matrix._21 = m_Transform.j.x;
	m_Matrix._22 = m_Transform.j.y;
	m_Matrix._23 = m_Transform.j.z;
	m_Matrix._24 = 0.0f;

	m_Matrix._31 = m_Transform.k.x;
	m_Matrix._32 = m_Transform.k.y;
	m_Matrix._33 = m_Transform.k.z;
	m_Matrix._34 = 0.0f;

	m_Matrix._41 = m_PostTransformOffset.x;
	m_Matrix._42 = m_PostTransformOffset.y;
	m_Matrix._43 = m_PostTransformOffset.z;
	m_Matrix._44 = 1.0f;
}

//------------------------------------------------------------------------------------
// EOF
