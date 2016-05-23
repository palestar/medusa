/*
	FactoryTypes.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/

#include "Reflection/TypeTemplate.h"

#include "ClassKey.h"
#include "WidgetKey.h"

//---------------------------------------------------------------------------------------------------

class InitializeFactoryTypes
{
public:
	InitializeFactoryTypes()
	{
		initialize();
	}

	static void initialize()
	{
		TypeSerialized<ClassKey>::instance();
		TypeCopyDefault<ClassKey,ClassKey>::instance();
		TypeCopyDefault<ClassKey,CharString>::instance();
		TypeCopyDefault<CharString,ClassKey>::instance();

		TypeSerialized<WidgetKey>::instance();
		TypeCopyDefault<WidgetKey,WidgetKey>::instance();
		TypeCopyDefault<WidgetKey,qword>::instance();
		TypeCopyDefault<qword,WidgetKey>::instance();
	}
} INIT_FACTORY_TYPES;

//---------------------------------------------------------------------------------------------------
//EOF
