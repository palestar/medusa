/*
	MathTypes.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/

#define MATH_DLL
#include "Reflection/TypeTemplate.h"

#include "Plane.h"
#include "Vector3.h"
#include "Matrix33.h"
#include "Quat.h"
#include "BoxHull.h"

//---------------------------------------------------------------------------------------------------

class InitializeMathTypes
{
public:
	InitializeMathTypes()
	{
		initialize();
	}

	static void initialize()
	{
		TypeSerialized<Vector3>::instance();
		TypeCopyDefault<Vector3,Vector3>::instance();
		TypeCopyDefault<Vector3,CharString>::instance();
		TypeCopyDefault<CharString,Vector3>::instance();

		TypeSerialized<Plane>::instance();
		TypeCopyDefault<Plane,Plane>::instance();
		TypeCopyDefault<Plane,CharString>::instance();
		TypeCopyDefault<CharString,Plane>::instance();

		TypeSerialized<Quat>::instance();
		TypeCopyDefault<Quat,Quat>::instance();
		TypeCopyDefault<Quat,CharString>::instance();
		TypeCopyDefault<CharString,Quat>::instance();

		TypeSerialized<Matrix33>::instance();
		TypeCopyDefault<Matrix33,Matrix33>::instance();
		TypeCopyDefault<Matrix33,CharString>::instance();
		TypeCopyDefault<CharString,Matrix33>::instance();

		TypeSerialized<BoxHull>::instance();
		TypeCopyDefault<BoxHull,BoxHull>::instance();
		TypeCopyDefault<BoxHull,CharString>::instance();
		TypeCopyDefault<CharString,BoxHull>::instance();
	}
} INIT_MATH_TYPES;

//---------------------------------------------------------------------------------------------------
//EOF
