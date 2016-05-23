/**
	@file NodePGE.h
	@brief Procedural Geometry Engine Node.

	This node provides a container for geometry created by the procedural geometry as well handles
	rendering this geometry in the scene graph of this engine.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/11/2011 4:06:55 PM
*/

#ifndef NODEPGE_H
#define NODEPGE_H

#include "NodeTransform.h"
#include "Render3dDll.h"

//---------------------------------------------------------------------------------------------------

class DLL NodePGE : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	//! A PGE node can have any number of parameter's, these objects define what will be created by the PGE node.
	class DLL Parameter : public Widget
	{
	public:
		DECLARE_WIDGET_CLASS();
		DECLARE_PROPERTY_LIST();

	};


	enum SubspaceBits {
		Q_RIGHT				= 0x0,
		Q_LEFT				= 0x1,
		Q_FRONT				= 0x0,
		Q_BACK				= 0x2,
		Q_TOP				= 0x0,
		Q_BOT				= 0x4,
	};

	//! This object is used to contain the geometry (verts, triangles, and materials) of a subspace. 
	class DLL Geometry : public Widget
	{

	};

	// sub-space quadrant positions, each  chunk can be divided into 8 subspace quadrants specified by the enum values below.
	enum SubspaceQuad
	{
		Q_RIGHT_FRONT_TOP	= Q_RIGHT|Q_FRONT|Q_TOP,		// 0
		Q_FIRST				= Q_RIGHT_FRONT_TOP,

		Q_LEFT_FRONT_TOP	= Q_LEFT|Q_FRONT|Q_TOP,			// 1
		Q_RIGHT_BACK_TOP	= Q_RIGHT|Q_BACK|Q_TOP,			// 2
		Q_LEFT_BACK_TOP		= Q_LEFT|Q_BACK|Q_TOP,			// 3
		Q_RIGHT_FRONT_BOT	= Q_RIGHT|Q_FRONT|Q_BOT,		// 4
		Q_LEFT_FRONT_BOT	= Q_LEFT|Q_FRONT|Q_BOT,			// 5
		Q_RIGHT_BACK_BOT	= Q_RIGHT|Q_BACK|Q_BOT,			// 6
		Q_LEFT_BACK_BOT		= Q_LEFT|Q_BACK|Q_BOT,			// 7
		Q_LAST				= Q_LEFT_BACK_BOT,

		Q_COUNT				// leave last always
	};

	//! This oct-tree node hold the geometry and links to child nodes for a given space within the NodePGE object space.
	class DLL Subspace : public Widget
	{
	public:
		DECLARE_WIDGET_CLASS();

		//! Types
		typedef Reference< Chunk >		Ref;

		//! Construction
		Subspace ( Subspace * a_pParent, SubspaceQuad a_eQuad );

		//! Accessors
		const BoxHull &			hull() const;			// position of this subspace relative to it's parent subspace
		float					scale() const;			// meters per unit (2.0 = 2 meters per 1, 0.01 = 1 cm per 1) within this sub-space
		Subspace *				parent() const;			// our parent subspace
		SubspaceQuad			quad() const;			// our quadrant in our parent subspace
		Subspace *				child( SubspaceQuad a_eQuad );   // get child subspace by quadrant


	};

};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
