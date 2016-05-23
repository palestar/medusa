/*
	Constant.h

	This class is used in place of a real constant value, allowing the constants of a program to be enumerated and changed
	allowing for more flexibility.

	Replace...
	const float		MIN_ZOOM = 2.0f;
	With...
	Constant		MIN_ZOOM( "MIN_ZOOM", 2.0f );

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef CONSTANT_H
#define CONSTANT_H

#include "Standard/Array.h"
#include "Standard/Tree.h"
#include "Standard/String.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

#pragma warning( disable: 4251 )		// disable dll-interface warning

class DLL Constant
{
public:
	// Constructors
	Constant( const char * pName, float value );
	~Constant();

	// Accessors
	const char *		name() const;
	float				value() const;
	operator			float() const;

	// Mutators
	Constant &			operator=( float value );

	// Static
	static Constant *	find( const char * pName );
	static int			constantCount();
	static Constant *	constant( int n );

	static void			registerConstant( Constant * pConstant );
	static void			unregisterConstant( Constant * pConstant );

private:
	// Data
	float				m_Value;
	const char *		m_pName;

	// Static
	static Array< Constant * >
						s_Constants;
	static Tree< CharString, Constant * >
						s_ConstantTree;
};

//----------------------------------------------------------------------------

inline Constant::operator float() const
{
	return m_Value;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF
