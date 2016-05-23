/**
	@file PGE.h
	@brief Procedural Geometry Engine

	This class will generate procedural geometry for rendering. This engine is initialize with N parameters, then 
	recursively generates geometry to N detail levels.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/11/2011 8:11:12 PM
*/

#ifndef PGE_H
#define PGE_H

#include "Factory/FactoryTypes.h"
#include "Render3dDll.h"

//---------------------------------------------------------------------------------------------------

class DLL PGE : public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	//! Base class of generated geometry
	class DLL Geometry : public Widget
	{
	public:
		DECLARE_WIDGET_CLASS();
		DECLARE_PROPERTY_LIST();

		//! Types
		typedef Reference< Geometry >		Ref;
		//! Construction
		Geometry();
		//! Interface
		virtual void		render( RenderContext & a_Context,
								const Matrix33 & frame, const Vector3 & position );
	protected:
		//! Types
		typedef std::list< DevicePrimitive::Ref >	PrimitiveList;
		//! Data
		PrimitiveList		m_RenderList;
	};
	typedef std::list< Geometry::Ref >		GeometryList;

	//! OctreeNode used for spatial subdivision of space
	class DLL OctreeNode : public Widget
	{
	public:
		//! Construction
		OctreeNode();
		OctreeNode( const BoxHull & a_hSpace, float a_fScale  );

		//! Accessors
		const BoxHull &		space() const;			//! This returns the space occupied by this node
		float				scale() const;			//! This returns the scale of space within this node
		float				rscale() const;			//! This returns the recipical of the scale, 1.0f / scale()
		OctreeNode *		parent() const;			//! Returns the parent node
		OctreeNode *		child( int n ) const;	//! Returns child 0-7
		const GeometryList&	geometry() const;		//! Return the geometry attached to this node

		//! Mutators

	protected:
		//! Data
		BoxHull				m_hSpace;
		float				m_fScale;
		float				m_fRscale;
		OctreeNode *		m_pParent;
		OctreeNode *		m_pChildren[ 8 ];	
		GeometryList		m_Geometry;				
	};

	//! 
	class DLL Viewer 
	{
	public:
		virtual void	getPosition( Vector3 & a_vPosition ) = 0;
	};

	//! This is the base class for a job object that is handled by the processing queue. 
	class DLL Job : public Widget
	{
	public:
		DECLARE_WIDGET_CLASS();
		DECLARE_PROPERTY_LIST();

		virtual u32		getCore() = 0;			//! Return the core index this job should run on, 0 for any core.			
		virtual u32		getPriority() = 0;			
	};

};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
