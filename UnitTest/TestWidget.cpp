/*
	TestWidget.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/

#include "UnitTest.h"
#include "Factory/FactoryTypes.h"
#include "File/FileMemory.h"

//---------------------------------------------------------------------------------------------------

enum TestEnum {
	ENUM1,
	ENUM2,
	ENUM3,
	ENUM4
};

IMPLEMENT_RAW_STREAMING( TestEnum );

enum TestBits {
	BIT1 = 0x1,
	BIT2 = 0x2,
	BIT3 = 0x4,
	BIT4 = 0x8
};

class MyWidget : public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< MyWidget >		Ref;

	// Data
	int			m_nInt;
	float		m_fFloat;
	CharString	m_sCharString;
	WideString	m_sWideString;

	Array< CharString >
				m_CharStringArray;
	Array< Ref >	
				m_WidgetArray;
	TestEnum	m_TestEnum;
	dword		m_TestBits;

	// Construction
	MyWidget() : m_nInt( 0 ), m_fFloat( 0 ), m_TestEnum( ENUM1 ), m_TestBits( 0 )
	{}
};

IMPLEMENT_FACTORY( MyWidget, Widget );
BEGIN_PROPERTY_LIST( MyWidget, Widget );
	ADD_PROPERTY( m_nInt );
	ADD_PROPERTY( m_fFloat );
	ADD_PROPERTY( m_sCharString );
	ADD_PROPERTY( m_sWideString );
	ADD_PROPERTY( m_CharStringArray );
	ADD_PROPERTY( m_WidgetArray );
	ADD_ENUM_PROPERTY( m_TestEnum );
		ADD_ENUM_OPTION( m_TestEnum, ENUM1 );
		ADD_ENUM_OPTION( m_TestEnum, ENUM2 );
		ADD_ENUM_OPTION( m_TestEnum, ENUM3 );
		ADD_ENUM_OPTION( m_TestEnum, ENUM4 );
	ADD_BITS_PROPERTY( m_TestBits );
		ADD_BIT_OPTION( m_TestBits, BIT1 );
		ADD_BIT_OPTION( m_TestBits, BIT2 );
		ADD_BIT_OPTION( m_TestBits, BIT3 );
		ADD_BIT_OPTION( m_TestBits, BIT4 );
END_PROPERTY_LIST();

//---------------------------------------------------------------------------------------------------

class TestWidget : public UnitTest
{
public:
	TestWidget( const char * pName ) : UnitTest( pName )
	{}

	void test()
	{
		MyWidget::Ref pSubObject = new MyWidget();
		pSubObject->m_nInt = 42;
		pSubObject->m_sCharString = "Hello World";
		pSubObject->m_sWideString = L"Hello Universe";
		pSubObject->m_CharStringArray.push( "Hello" );
		pSubObject->m_CharStringArray.push( "World" );

		MyWidget::Ref pObject = new MyWidget();
		pObject->m_nInt = 23;
		pObject->m_sCharString = "Hello World";
		pObject->m_sWideString = L"Hello Universe";
		pObject->m_CharStringArray.push( "Hello" );
		pObject->m_CharStringArray.push( "World" );
		pObject->m_WidgetArray.push( pSubObject );

		// Test property system...
		Test( pObject->set( "m_fFloat", 64 ) );
		Test( pObject->m_fFloat == 64.0f );

		// Test enum conversions
		Test( pObject->set( "m_TestEnum", "ENUM2" ) );
		Test( pObject->m_TestEnum == ENUM2 );
		CharString sValue;
		Test( pObject->get( "m_TestEnum", sValue ) );
		Test( sValue == "ENUM2" );
		unsigned int nValue;
		Test( pObject->get( "m_TestEnum", nValue ) );
		Test( nValue == ENUM2 );
		TestEnum eValue;
		Test( pObject->get( "m_TestEnum", eValue ) );
		Test( eValue == ENUM2 );

		// Test bit conversions
		Test( pObject->set( "m_TestBits", "BIT1|BIT2|BIT4" ) );
		Test( pObject->m_TestBits == (BIT1|BIT2|BIT4) );
		Test( pObject->get( "m_TestBits", sValue ) );
		Test( sValue == "BIT1|BIT2|BIT4" );
		Test( pObject->get( "m_TestBits", nValue ) );
		Test( nValue == (BIT1|BIT2|BIT4) );


		// serialize the widget to a memory block
		FileMemory::Ref pFile = new FileMemory();
		OutStream( pFile ) << pObject;
		
		// free the object
		pObject = NULL;
		pSubObject = NULL;

		// deserialize the widget
		pFile->rewind();
		InStream( pFile ) >> pObject;

		Test( pObject->m_nInt == 23 );
		Test( pObject->m_sCharString == "Hello World" );
		Test( pObject->m_sWideString == L"Hello Universe" );
		Test( pObject->m_CharStringArray.size() == 2 );
		Test( pObject->m_WidgetArray.size() == 1 );

		pSubObject = pObject->m_WidgetArray[ 0 ];
		Test( pSubObject->m_nInt == 42 );
		Test( pSubObject->m_sCharString == "Hello World" );
		Test( pSubObject->m_sWideString == L"Hello Universe" );
		Test( pSubObject->m_CharStringArray.size() == 2 );

		// test Text streaming
		pFile->rewind();
		pFile->setEOF();

		OutStream( pFile, FF_TEXT ) << pObject;

		// free the object
		pObject = NULL;
		pSubObject = NULL;

		// deserialize the widget
		pFile->rewind();
		InStream( pFile, FF_TEXT ) >> pObject;

		Test( pObject->m_nInt == 23 );
		Test( pObject->m_sCharString == "Hello World" );
		Test( pObject->m_sWideString == L"Hello Universe" );
		Test( pObject->m_CharStringArray.size() == 2 );
		Test( pObject->m_WidgetArray.size() == 1 );

		pSubObject = pObject->m_WidgetArray[ 0 ];
		Test( pSubObject->m_nInt == 42 );
		Test( pSubObject->m_sCharString == "Hello World" );
		Test( pSubObject->m_sWideString == L"Hello Universe" );
		Test( pSubObject->m_CharStringArray.size() == 2 );
	}

} TEST_WIDGET( "TestWidget" );

//---------------------------------------------------------------------------------------------------
//EOF
