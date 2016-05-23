/*
	TargetReference.h

	This smart pointer is used to point to either a NOunTarget type object, or a Widget object. If it is pointing to a NounTarget
	type object and is serialized, then it serializes the entire object, otherwise it serializes the WidgetKey of Widget
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef TARGETREFERENCE_H
#define TARGETREFERENCE_H

#include "Factory/FactoryTypes.h"

// forward declare
class NounTarget;

//----------------------------------------------------------------------------

template<class T>
class TargetReference : public WidgetReference<T>
{
public:
	// Construction
	TargetReference()
	{}
	TargetReference( T * pPointer ) : WidgetReference<T>( pPointer )
	{}
	TargetReference( const TargetReference<T> & assign ) : WidgetReference<T>( assign )
	{}
	TargetReference( const WidgetKey & key ) : WidgetReference<T>( key )
	{}
};

//----------------------------------------------------------------------------

template<class T>
inline const OutStream & operator<<( const OutStream &output, const TargetReference<T> & ref )
{
	if ( WidgetCast<NounTarget>( ref.pointer() ) )
	{
		output << true;

		dword nFilter = const_cast<OutStream &>(output).clearFilter( FF_UPDATE );
		output << ref.pointer();
		const_cast<OutStream &>(output).setFilter( nFilter );
	}
	else
	{
		output << false;
		output << ref.key();
	}

	return output;
}

template<class T>
inline const InStream & operator>>( const InStream &input, TargetReference<T> &ref )
{
	bool bTarget;
	input >> bTarget;

	if ( bTarget )
	{
		dword nFilter = const_cast<InStream&>(input).clearFilter( FF_UPDATE );
		NounTarget * pTarget = (NounTarget *)ref.pointer();
		input >> pTarget;
		const_cast<InStream&>(input).setFilter( nFilter );

		ref = (T *)pTarget; //.setPointer( pTarget );
	}
	else
	{
		WidgetKey nKey( (qword)0 );
		input >> nKey;

		ref.setKey( nKey );
	}

	return input;
}

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF
