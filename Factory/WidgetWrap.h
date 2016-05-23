/*
	WidgetWrap.h

	This object is used to package a widget for serialization, without having
	to create a instance of the object when deserilize

	(c)2005 Palestar, Richard Lyle
*/

#ifndef WIDGET_WRAP_H
#define WIDGET_WRAP_H

#include "Standard/Types.h"
#include "File/Stream.h"
#include "Factory/FactoryTypes.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL WidgetWrap
{
public:
	// Construction
	WidgetWrap();
	WidgetWrap( const WidgetWrap & copy );
	WidgetWrap( Widget * pWidget, dword nFilter, Dictionary * a_pDictionary );
	WidgetWrap( Widget * pWidget, dword nFilter );
	WidgetWrap( Widget * pWidget );
	~WidgetWrap();

	// Accessors
	dword			filter() const;
	Dictionary *	dictionary() const;
	int				size() const;
	byte *			widget() const;
	Widget *		unwrap( Widget * pWidget = NULL ) const;
	
	CharString		uuencode() const;					// encode binary data into a text string

	// Mutators
	void			setFilter( dword a_nFilter );
	void			setDictionary( Dictionary * a_pDict );
	void			clearDictionary();

	bool			uudecode( const char * pText );		// decode binary data from a text string
	void			wrap( dword nFilter, Dictionary * a_pDictionary, int size, byte * pWidget, bool copy = false );	
	void			wrap( Widget * pWidget );
	void			wrap( Widget * pWidget, dword nFilter );
	void			wrap( Widget * pWidget, dword nFilter, Dictionary * a_pDictionary );
	void			grab( WidgetWrap & grab );			// take the wrapped widget away from another WidgetWrap object
	void			release();

	WidgetWrap &	operator=( const WidgetWrap & copy );
	WidgetWrap &	operator=( Widget * pWidget );

private:
	// Data
	dword			m_nFilter;						// filter flags used to wrap the object
	Dictionary::Ref	m_pDictionary;					// dictionary used to wrap/unwrap the object
	int				m_Size;							// compressed size
	byte *			m_pWidget;
};

//----------------------------------------------------------------------------

inline dword WidgetWrap::filter() const
{
	return m_nFilter;
}

inline Dictionary * WidgetWrap::dictionary() const
{
	return m_pDictionary;
}

inline int WidgetWrap::size() const
{
	return m_Size;
}

inline byte * WidgetWrap::widget() const
{
	return m_pWidget;
}

inline const InStream & operator>>( const InStream & input, WidgetWrap & wrap )
{
	int size;
	input >> size;

	if ( size > 0 )
	{
		byte nFilter;
		input >> nFilter;

		byte * pWidget = new byte[ size ];
		input.read( pWidget, size );

		wrap.wrap( nFilter, input.dictionary(), size, pWidget, false );
	}

	return input;
}

inline const OutStream & operator<<( const OutStream & output, const WidgetWrap & wrap )
{
	output << wrap.size();
	if ( wrap.size() > 0 )
	{
		output << (byte)wrap.filter();
		output.write( wrap.widget(), wrap.size() );
	}

	return output;
}

//---------------------------------------------------------------------------------------------------

inline void WidgetWrap::setFilter( dword a_nFilter )
{
	m_nFilter = a_nFilter;
}

inline void WidgetWrap::setDictionary( Dictionary * a_pDict )
{
	m_pDictionary = a_pDict;
}

inline void WidgetWrap::clearDictionary()
{
	m_pDictionary = NULL;
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
