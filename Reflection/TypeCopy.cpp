/*
	TypeCopy.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/

#pragma warning( disable : 4244 )		// possible loss of data warning
#pragma warning( disable : 4800 )		// forcing value to bool 'true' or 'false' (performance warning)
#pragma warning( disable : 4996 )		// This function or variable may be unsafe. Consider using sscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

#include "Reflection/TypeCopy.h"
#include "Reflection/Type.h"
#include "Standard/Color.h"
#include "Standard/UniqueNumber.h"

//---------------------------------------------------------------------------------------------------

TypeCopy::TypeCopy( Type * pLeft, Type * pRight ) 
	: m_pLeft( pLeft ), m_pRight( pRight ), m_nHash( 0 )
{
	if ( m_pLeft == NULL || m_pRight == NULL )
		return;		// one or more invalid type id's
	m_nHash = (((qword)m_pLeft->typeId()) << 32) | m_pRight->typeId();
	if ( typeCopyHash().find( m_nHash ).valid() )
		return;		// this TypeCopyTemplate already exists

	typeCopyHash()[ m_nHash ] = this;
	typeCopyGraph().addLink( m_pRight->typeId(), m_pLeft->typeId() );
	// destroy any cached conversion paths, because we may now have a better one...
	typePathHash().release();
}

TypeCopy::~TypeCopy()
{}

//----------------------------------------------------------------------------

TypeCopy * TypeCopy::find( int nLeftId, int nRightId )
{
	qword nHash = (((qword)nLeftId) << 32) | nRightId;
	TypeCopyHash::Iterator iType = typeCopyHash().find( nHash );
	if ( iType.valid() )
		return *iType;

	return NULL;
}

Type * TypeCopy::findBestType( const Array< Type * > & LeftTypes, Type * pRightType )
{
	if ( pRightType == NULL )
		return NULL;		

	Type * pBestType = NULL;
	int nBestPath = 0;

	for(int i=0;i<LeftTypes.size();i++)
	{
		Type * pLeftType = LeftTypes[ i ];
		if ( pLeftType == NULL )
			continue;					// invalid type
		if ( pLeftType->typeId() == pRightType->typeId() )
			return pLeftType;			// of the same type, this is always the closest type...

		Array< int > path;
		if (! findPath( pLeftType, pRightType, path ) )
			continue;					// no conversion path...

		if ( pBestType == NULL || path.size() < nBestPath )
		{
			pBestType = pLeftType;
			nBestPath = path.size();
		}
	}

	return pBestType;
}

bool TypeCopy::copy( void * pLeft, int nLeftType, const void * pRight, int nRightType )
{
	return copy( pLeft, Type::find( nLeftType ), pRight, Type::find( nRightType ) );
}

bool TypeCopy::copy( void * pLeft, const Type * pLeftType, const void * pRight, const Type * pRightType )
{
	if ( pLeft == NULL || pLeftType == NULL || pRight == NULL || pRightType == NULL )
		return false;

	// look for a direct type conversion first, then look for an indirect type conversion if none is found...
	TypeCopy * pCopy = find( pLeftType->typeId(), pRightType->typeId() );
	if ( pCopy != NULL )
		return pCopy->copy( pLeft, pRight );

	Array< int > path;
	if (! findPath( pLeftType, pRightType, path ) )
		return false;		// no conversion path found!

	// we found a conversion path between our right type and left type, convert the data from
	// one type to the next until we can finally convert it into our left type
	void * pLast = const_cast<void *>( pRight );
	Type * pLastType = const_cast<Type *>( pRightType );

	bool bFreeLast = false;

	// now convert into the correct type
	for(int i=0;i<(int)path.size();i++)
	{
		Type * pNewType = Type::find( path[i] );
		if ( pNewType == NULL || pNewType == pLeftType )
			break;			// failed to find the new type 
		if ( pNewType == pLeftType )
			break;			// the next type is our left type, break out and do a direct conversion now
		if ( pNewType == pLastType )
			continue;		// data is already of this type, continue to the next type

		// now copy from the last type into the new type
		TypeCopy * pCopy = find( pNewType->typeId(), pLastType->typeId() );
		if ( pCopy == NULL )
			break;

		// allocate a buffer for the converted data
		void * pNew = pNewType->allocate();
		// convert the data from the last type into the new type
		if (! pCopy->copy( pNew, pLast ) )
		{
			pNewType->free( pNew );
			break;
		}
		if ( bFreeLast )
			pLastType->free( pLast );

		bFreeLast = true;
		pLast = pNew;
		pLastType = pNewType;
	}

	bool bSuccess = false;

	// we should now be able to perform a direct copy to our new type
	pCopy = find( pLeftType->typeId(), pLastType->typeId() );
	if ( pCopy != NULL )
		bSuccess = pCopy->copy( pLeft, pLast );

	if ( bFreeLast )
		pLastType->free( pLast );

	return bSuccess;
}

//----------------------------------------------------------------------------

bool TypeCopy::findPath( const Type * pLeftType, const Type * pRightType, Array< int > & path )
{
	qword nHash = (((qword)pLeftType->typeId()) << 32) | pRightType->typeId();
	TypePathHash::Iterator iPath = typePathHash().find( nHash );
	if ( iPath.valid() )
	{
		// found a cached path, return the previously found conversion path...
		path = *iPath;
		return true;
	}

	// enumerate the TypeCopy objects, and determine if we can indirectly convert the types..
	// We do this by using an A* path-finding algo to find the best path between the source type (right)
	// and the destionation type (left).
	int nCost = typeCopyGraph().findPath( pRightType->typeId(), pLeftType->typeId(), path );
	if ( nCost < 0 )
		return false;		// no path was found

	// cache the path for next time
	typePathHash()[ nHash ] = path;
	return true;
}

//---------------------------------------------------------------------------------------------------

class CopyColorString : public TypeCopyTemplate<CopyColorString,CharString,Color>
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		((CharString *)pLeft)->format( "%8.8X", (*((Color *)pRight)).RGBA() );
		return true;
	}
};

class CopyStringColor : public TypeCopyTemplate<CopyStringColor,Color,CharString>			// Color <- CharString
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((Color *)pLeft) = Color( strtoul( *((CharString *)pRight), NULL, 16 ) );
		return true;
	}
};

class CopyRectIntString : public TypeCopyTemplate< CopyRectIntString, CharString, RectInt>
{
public:
   bool copy( void * pLeft, const void * pRight )
   {
      ((CharString *)pLeft)->format( "%d,%d,%d,%d", 
         (*((RectInt *)pRight)).left,
         (*((RectInt *)pRight)).top,
         (*((RectInt *)pRight)).right,
         (*((RectInt *)pRight)).bottom );
      return true;
   }
};

class CopyStringRectInt : public TypeCopyTemplate<CopyStringRectInt,RectInt,CharString>
{
public:
   bool copy( void * pLeft, const void * pRight )
   {
      sscanf( (*((CharString*)pRight)).cstr(), "%d,%d,%d,%d", 
         &((RectInt *)pLeft)->left,
         &((RectInt *)pLeft)->top, 
         &((RectInt *)pLeft)->right,
         &((RectInt *)pLeft)->bottom );
      return true;
   }
};

class CopyRectFloatString : public TypeCopyTemplate< CopyRectFloatString, CharString, RectFloat>
{
public:
   bool copy( void * pLeft, const void * pRight )
   {
      ((CharString *)pLeft)->format( "%f,%f,%f,%f", 
         (*((RectFloat *)pRight)).left,
         (*((RectFloat *)pRight)).top,
         (*((RectFloat *)pRight)).right,
         (*((RectFloat *)pRight)).bottom );
      return true;
   }
};

class CopyStringRectFloat : public TypeCopyTemplate<CopyStringRectFloat,RectFloat,CharString>
{
public:
   bool copy( void * pLeft, const void * pRight )
   {
      sscanf( (*((CharString*)pRight)).cstr(), "%f,%f,%f,%f", 
         &((RectFloat *)pLeft)->left,
         &((RectFloat *)pLeft)->top, 
         &((RectFloat *)pLeft)->right,
         &((RectFloat *)pLeft)->bottom );
      return true;
   }
};

class CopyFloatString : public TypeCopyTemplate<CopyFloatString,CharString,float>
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((CharString *)pLeft) = CharString().format( "%f", *((float *)pRight) );
		return true;
	}
};

class CopyDoubleString : public TypeCopyTemplate<CopyDoubleString,CharString,double>
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((CharString *)pLeft) = CharString().format( "%lf", *((double *)pRight) );
		return true;
	}
};

class CopyIntString : public TypeCopyTemplate<CopyIntString,CharString,int>
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((CharString *)pLeft) = CharString().format( "%d", *((int *)pRight) );
		return true;
	}
};

class CopyDWORDString : public TypeCopyTemplate<CopyDWORDString,CharString,dword>
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((CharString *)pLeft) = CharString().format( "%u", *((dword *)pRight) );
		return true;
	}
};

class CopyQWORDString : public TypeCopyTemplate<CopyQWORDString,CharString,qword>
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((CharString *)pLeft) = CharString().format( "%llu", *((qword *)pRight) );
		return true;
	}
};

class CopyStringFloat : public TypeCopyTemplate<CopyStringFloat,float,CharString>			// float <- CharString
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((float *)pLeft) = (float)strtod( (*((CharString *)pRight)), NULL );
		return true;
	}
};

class CopyStringDouble : public TypeCopyTemplate<CopyStringDouble,double,CharString>			// float <- CharString
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((double *)pLeft) = strtod( (*((CharString *)pRight)), NULL );
		return true;
	}
};

class CopyStringInteger : public TypeCopyTemplate<CopyStringInteger,int,CharString>			// int <- CharString
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((int *)pLeft) = strtol( (*((CharString *)pRight)), NULL, 10 );
		return true;
	}
};

class CopyStringDWORD : public TypeCopyTemplate<CopyStringDWORD,dword,CharString>			// dword <- CharString
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((dword *)pLeft) = strtoul( (*((CharString *)pRight)), NULL, 10 );
		return true;
	}
};

class CopyStringQWORD : public TypeCopyTemplate<CopyStringQWORD,qword,CharString>			// qword <- CharString
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		qword n = 0;
		sscanf( (*((CharString *)pRight)), "%llu", &n );
		*((qword *)pLeft) = n;
		return true;
	}
};

class CopyStringWideString : public TypeCopyTemplate<CopyStringWideString,WideString,CharString>	// WideString <- CharString
{
public:
	bool copy( void * pLeft, const void * pRight )
	{
		*((WideString *)pLeft) = ( *((CharString *)pRight) );
		return true;
	}
};

class CopyWideStringString : public TypeCopyTemplate<CopyWideStringString,CharString,WideString>	// CharString <- WideString
{
	bool copy( void * pLeft, const void * pRight )
	{
		*((CharString *)pLeft) = ( *((WideString *)pRight) );
		return true;
	}
};

class CopyCharString : public TypeCopyTemplate<CopyCharString,const char *,CharString >
{
	bool copy( void * pLeft, const void * pRight )
	{
		*((const char **)pRight) = ((CharString *)pRight)->buffer();
		return true;
	}
};

class CopyWideCharWideString : public TypeCopyTemplate<CopyWideCharWideString,const wchar *,WideString >
{
	bool copy( void * pLeft, const void * pRight )
	{
		*((const wchar **)pRight) = ((WideString *)pRight)->buffer();
		return true;
	}
};

//---------------------------------------------------------------------------------------------------

void TypeCopy::initialize()
{
	Type::initialize();	

	TypeCopyDefault<u8,u8>::instance();								// u8
	TypeCopyDefault<u8,u32>::instance();
	TypeCopyDefault<u32,u8>::instance();
	TypeCopyDefault<s8,s8>::instance();								// s8
	TypeCopyDefault<s8,s32>::instance();
	TypeCopyDefault<s32,s8>::instance();
	TypeCopyDefault<u16,u16>::instance();							// u16
	TypeCopyDefault<u16,u32>::instance();
	TypeCopyDefault<u32,u16>::instance();
	TypeCopyDefault<s16,s16>::instance();							// s16
	TypeCopyDefault<s16,s32>::instance();
	TypeCopyDefault<s32,s16>::instance();
	TypeCopyDefault<u32,u32>::instance();							// u32
	TypeCopyDefault<s32,s32>::instance();							// s32
	TypeCopyDefault<ul32,ul32>::instance();							// ul32
	TypeCopyDefault<ul32,u32>::instance();
	TypeCopyDefault<u32,ul32>::instance();
	TypeCopyDefault<sl32,sl32>::instance();							// sl32
	TypeCopyDefault<sl32,s32>::instance();
	TypeCopyDefault<s32,sl32>::instance();

	TypeCopyDefault<u64,u64>::instance();							// u64
	TypeCopyDefault<u64,u32>::instance();			
	TypeCopyDefault<s64,s64>::instance();							// s64
	TypeCopyDefault<s64,s32>::instance();

	TypeCopyDefault<float,float>::instance();
	TypeCopyDefault<double,float>::instance();
	TypeCopyDefault<float,double>::instance();
	TypeCopyDefault<double,double>::instance();

	TypeCopyDefault<bool,bool>::instance();							// bool
	TypeCopyDefault<bool,u8>::instance();
	TypeCopyDefault<u8,bool>::instance();

	TypeCopyDefault<CharString,CharString>::instance();				// CharString
	TypeCopyDefault<CharString,const char *>::instance();
	CopyWideStringString::instance();
	CopyFloatString::instance();									// CharString <- float
	CopyDoubleString::instance();									// CharString <- double
	CopyIntString::instance();										// CharString <- int
	CopyDWORDString::instance();									// CharString <- dword
	CopyQWORDString::instance();									// CharString <- qword
	CopyStringFloat::instance();									// CharString -> float
	CopyStringDouble::instance();									// CharString -> double
	CopyStringInteger::instance();									// CharString -> int
	CopyStringDWORD::instance();									// CharString -> dword
	CopyStringQWORD::instance();									// CharString -> qword
	CopyCharString::instance();										// CharString -> const char *

	TypeCopyDefault<WideString,WideString>::instance();				// WideString
	TypeCopyDefault<WideString,const wchar *>::instance();
	CopyStringWideString::instance();	
	CopyWideCharWideString::instance();								// WideString -> const wchar *

	TypeCopyDefault<Color,Color>::instance();
	TypeCopyDefault<Color,u32>::instance();
	TypeCopyDefault<u32,Color>::instance();
	CopyColorString::instance();									// Color
	CopyStringColor::instance();
	CopyRectIntString::instance();                           // RectInt -> CharString
	CopyStringRectInt::instance();                           // CharString -> RectInt
	CopyRectFloatString::instance();                         // RectFloat -> CharString
	CopyStringRectFloat::instance();                         // CharString -> RectFloat

	TypeCopyDefault<UniqueNumber,UniqueNumber>::instance();
	TypeCopyDefault<qword,UniqueNumber>::instance();
	TypeCopyDefault<UniqueNumber,qword>::instance();

	TypeCopyDefault<SizeInt,SizeInt>::instance();
	TypeCopyDefault<SizeInt,SizeFloat>::instance();
	TypeCopyDefault<SizeFloat,SizeInt>::instance();
}

void TypeCopy::release()
{
	typeCopyHash().release();
	typeCopyGraph().release();
	typePathHash().release();
}

//---------------------------------------------------------------------------------------------------

TypeCopy::TypeCopyHash & TypeCopy::typeCopyHash()
{
	static TypeCopyHash HASH;
	return HASH;
}

TypeCopy::TypeCopyGraph & TypeCopy::typeCopyGraph()
{
	static TypeCopyGraph HASH;
	return HASH;
}

TypeCopy::TypePathHash & TypeCopy::typePathHash()
{	
	static TypePathHash HASH;
	return HASH;
}

//---------------------------------------------------------------------------------------------------

// Helper class to automatically initialize the TypeCopy system when in a DLL
class InitializeTypeCopy
{
public:
	InitializeTypeCopy()
	{
		TypeCopy::initialize();
	}
} TYPE_COPY_INIT;

//---------------------------------------------------------------------------------------------------
//EOF
