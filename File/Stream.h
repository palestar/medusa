/*
	Stream.h

	The Stream class is the base class for the InStream / OutStream classes. The Stream classes wrap
	a "File" object, and handle the reading / writing of data to an file object.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include "File.h"
#include "Dictionary.h"

#include "Standard/Types.h"				// Data Types that need streaming
#include "Standard/UniqueNumber.h"
#include "Standard/WidgetKey.h"
#include "Standard/String.h"
#include "Standard/CharString.h"
#include "Standard/Color.h"
#include "Standard/Size.h"
#include "Standard/Point.h"
#include "Standard/Rect.h"
#include "Standard/Array.h"
#include "Standard/SortedArray.h"
#include "Standard/Tree.h"
#include "Standard/Hash.h"
#include "Standard/HashArray.h"
#include "Standard/Reference.h"
#include "Standard/List.h"
#include "Standard/Queue.h"
#include "Standard/Buffer.h"

#include <vector>
#include <list>
#include <map>

#include "MedusaDll.h"

//-------------------------------------------------------------------------------------------------

enum StreamVersion {
	// Stream version numbers MUST go up in value for most of the streaming versioning to work.. many routines
	// do a simply if input.version() < SV_20090104 check.
	SV_UNKNOWN			= 0x0,
	SV_20090104			= 0x20090104,
	SV_CURRENT			= SV_20090104,
};

// flags used by Stream::filter()
enum FilterFlags {	
	// Serialization flags
	FF_COPY				= 0x00000001,		// object is being copied, so ignore properties with PF_NOCOPY flag
	FF_TRANSMIT			= 0x00000002,		// object is being transmitted, so send properties with the PF_TRANSMIT flag
	FF_UPDATE			= 0x00000004,		// object is being updating a previously serialized object, serialize properties with PF_UPDATE only
	FF_NOOBJECTS		= 0x00000008,		// Use to perform a shallow serialization of a object, if a property has the PF_OBJECT flag it will be skipped
	FF_SHARED			= 0x00000010,		// Use this flag to load Widget's into any existing Widget
	FF_TEXT				= 0x80000000,		// stream should write out a readable text format instead of binary
};

/*
	Stream base class, InStream and OutStream are derived from this object

    - The filter flags are user defined flags for controlling what is sent/received through the stream. 
	
	For example, Widget::read() virtual takes only an InStream object, the read() function of your widget
	can use the flags stored in the stream to filter out certain data, or add in data not normally included
	in the serialization of the object. The class BaseNode, uses a filter flag to control if it serialized
	it's child objects or not, useful in network conditions when you may not want to send the entire hierarchy
	to a client.

*/
class DLL Stream	// base class only!
{
public:
	// Construction
	virtual	~Stream();

	// Accessors
	File *				file() const;					// get the file object
	dword				filter() const;					// get the filter flags
	int					version() const;				// get the file version
	Dictionary *		dictionary() const;				// get the dictionary for this stream

	// Mutators
	void				setFile( File * pFile );		// set the file object to use
	dword				setFilter( dword filter );		// set the filter flags, returns the previous flags
	dword				clearFilter( dword nClear );	// remove the specific filter flags, returns the flags before clearing
	dword				addFilter( dword nAdd );		// add the specified filter flags, returns the flags before adding
	void				setVersion( int a_nVersion );	// set the stream version
	void				setDictionary( Dictionary * a_pDictionary );

	// Helpers
	bool				setPosition( File::Position a_nPosition );

protected:
	// Protected Constructors
	Stream( const Stream &copy );
	Stream( File * pFile, dword a_nFilter = 0, int a_nVersion = SV_CURRENT, Dictionary * a_pDict = NULL );

protected:
	// Data
	File::Ref			m_pFile;
	dword				m_nFilter;
	int					m_nVersion;
	Dictionary::Ref		m_pDictionary;
}; 

//----------------------------------------------------------------------------

inline File * Stream::file() const
{ 
	return m_pFile; 
}

inline dword Stream::filter() const
{
	return m_nFilter;
}

inline int Stream::version() const
{
	return m_nVersion;
}

inline Dictionary * Stream::dictionary() const
{
	return m_pDictionary;
}

//---------------------------------------------------------------------------------------------------

inline bool Stream::setPosition( File::Position a_nPosition )
{
	if (! m_pFile.valid() )
		return false;
	return m_pFile->setPosition( a_nPosition ) == a_nPosition;
}

//-------------------------------------------------------------------------------------------------

class DLL InStream : public Stream
{
public :
	// Construction
	InStream( File * pFile, dword a_nFilter = 0, int a_nVersion = SV_CURRENT, Dictionary * a_pDict = NULL ); 	
	InStream( const InStream & copy );
	virtual ~InStream();

	int					bytes() const;									// returns the number of bytes remaining in the stream
	void				read( void * pDst, int bytes ) const;			// read in binary data
	const char *		readLine( bool a_bEscape = false ) const;		// read a line of text, the pointer to the line is only valid until the next call to readLine()
	void				readHexLine( void * pDst, int bytes ) const;	// read a line of hex data into the given buffer

private:
	mutable char *		m_pLineBuffer;
	mutable int			m_nLineBufferMax;
};

//-------------------------------------------------------------------------------------------------

class DLL OutStream : public Stream
{
public:	
	// Construction
	OutStream( File * pFile, dword a_nFilter = 0, int a_nVersion = SV_CURRENT, Dictionary * a_pDict = NULL ); 
	OutStream( File * pFile, byte * pWrite, dword bytes, dword a_nFilter = 0, int a_nVersion = SV_CURRENT, Dictionary * a_pDict = NULL );
	OutStream( const OutStream & copy );
	~OutStream();

	void				pipe( const InStream & input ) const;
	void				write( const void * pSrc, int bytes ) const;					// write bytes
	void				writeLine( const char * pLine, bool a_bEscape = false ) const;	// write a line of text
	void				writeHexLine( const void * pSrc, int bytes ) const;				// convert the binary data into hex and write out the line
	void				writeFormatted( const char * pFormat, ... ) const;
	void				flush() const;													// flush the file

	static char *		escapeString( const char * a_pIn );

private:
	mutable char *		m_pFormatBuffer;
	mutable int			m_nFormatBufferMax;
}; 

//-------------------------------------------------------------------------------------------------

// Use this macro to quickly implement raw byte streaming of your class or structure, do not
// use this on a class that contains pointers 

#define IMPLEMENT_RAW_STREAMING( CLASS )																	\
	inline const InStream & operator>>( const InStream & input, CLASS & object )							\
	{																										\
		if ( (input.filter() & FF_TEXT) == 0 )																\
			input.read( &object, sizeof( CLASS ) );															\
		else																								\
			input.readHexLine( &object, sizeof(CLASS) );													\
		return input;																						\
	}																										\
	inline const OutStream & operator<<( const OutStream & output, const CLASS & object)					\
	{																										\
		if ( (output.filter() & FF_TEXT) == 0 )																\
			output.write( &object, sizeof( CLASS ) );														\
		else																								\
			output.writeHexLine( &object, sizeof(CLASS) );													\
		return output;																						\
	}

#define IMPLEMENT_FRIEND_STREAMING( CLASS )																	\
	friend const InStream & operator>>( const InStream & input, CLASS &object)								\
	{																										\
		if ( (input.filter() & FF_TEXT) == 0 )																\
			input.read( &object, sizeof( CLASS ) );															\
		else																								\
			input.readHexLine( &object, sizeof(CLASS) );													\
		return( input );																					\
	}																										\
	friend const OutStream & operator<<( const OutStream & output, const CLASS &object)						\
	{																										\
		if ( (output.filter() & FF_TEXT) == 0 )																\
			output.write( &object, sizeof( CLASS ) );														\
		else																								\
			output.writeHexLine( &object, sizeof(CLASS) );													\
		return( output );																					\
	}

//---------------------------------------------------------------------------------------------------

IMPLEMENT_RAW_STREAMING( UniqueNumber );
IMPLEMENT_RAW_STREAMING( Color );

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, u8 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		data = (u8)strtoul( input.readLine(), NULL, 10 );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const u8 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%u", (u32)data );
	return output;																		
}

inline const InStream & operator>>( const InStream & input, u16 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		data = (u16)strtoul( input.readLine(), NULL, 10 );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const u16 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%u", (u32)data );
	return output;																		
}

inline const InStream & operator>>( const InStream & input, u32 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		data = (u32)strtoul( input.readLine(), NULL, 10 );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const u32 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%u", data );
	return output;																		
}

inline const InStream & operator>>( const InStream & input, u64 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		sscanf( input.readLine(), "%llu", &data );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const u64 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%llu", data );
	return output;																		
}

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, s8 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		data = (s8)atoi( input.readLine() );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const s8 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%d", (int)data );
	return output;																		
}

inline const InStream & operator>>( const InStream & input, s16 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		data = (s16)atoi( input.readLine() );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const s16 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%d", (int)data );
	return output;																		
}

inline const InStream & operator>>( const InStream & input, s32 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		data = atoi( input.readLine() );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const s32 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%d", data );
	return output;																		
}

inline const InStream & operator>>( const InStream & input, s64 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		sscanf( input.readLine(), "%lld", &data );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const s64 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%lld", data );
	return output;																		
}

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, f32 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		sscanf( input.readLine(), "%f", &data );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const f32 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%f", data );
	return output;																		
}

inline const InStream & operator>>( const InStream & input, f64 & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		sscanf( input.readLine(), "%lf", &data );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const f64 & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%lf", data );
	return output;																		
}

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, bool & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		data = atoi( input.readLine() ) != 0;
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const bool & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%d", (u32)data );
	return output;																		
}

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, unsigned long & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		data = strtoul( input.readLine(), NULL, 10 );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const unsigned long & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%u", data );
	return output;																		
}

inline const InStream & operator>>( const InStream & input, long & data )
{																		
	if ( (input.filter() & FF_TEXT) == 0 )								
		input.read( &data, sizeof( data ) );							
	else							
		data = atoi( input.readLine() );
	return input;														
}																		

inline const OutStream & operator<<( const OutStream & output, const long & data )	
{																						
	if ( (output.filter() & FF_TEXT) == 0 )												
		output.write( &data, sizeof( data ) );										
	else																				
		output.writeFormatted( "%d", data );
	return output;																		
}

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, CharString & s )
{
	if ( (input.filter() & FF_TEXT) == 0 )												
	{
		int nLength;
		input >> nLength;
		
		if ( nLength & 0x80000000 )
		{
			nLength &= 0x7FFFFFFF;

			if ( nLength > 0 )
			{
				WideString ws;
				ws.allocate( nLength );

				input.read( ws.buffer(), nLength * sizeof(wchar) );

				// Convert from UNICODE to ANSI
				s.copy( ws );
			}
		}
		else if ( nLength > 0 )
		{
			s.allocate( nLength );
			input.read( s.buffer(), sizeof(char) * nLength );
		}
	}
	else
		s = input.readLine( true );

	return input;
}

inline const OutStream & operator<<( const OutStream & output, const CharString & s )
{
	if ( (output.filter() & FF_TEXT) == 0 )												
	{
		int nLength = s.length();
		output.write( &nLength, sizeof(int) );
		output.write( s.buffer(), nLength * sizeof(char) );
	}
	else
		output.writeLine( s, true );

	return output;
}


//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, WideString & s )
{
	if ( (input.filter() & FF_TEXT) == 0 )												
	{
		int nLength;
		input >> nLength;
		
		if ( nLength & 0x80000000 )
		{
			nLength &= 0x7FFFFFFF;
			if ( nLength > 0 )
			{
				s.allocate( nLength );
				input.read( s.buffer(), sizeof(wchar) * nLength );
			}
		}
		else if ( nLength > 0 )
		{
			CharString cs;
			cs.allocate( nLength );

			input.read( cs.buffer(), sizeof(char) * nLength );

			// convert from ANSI to UNICODE
			s.copy( cs );
		}
	}
	else
	{
		int nLength;
		input >> nLength;

		s.allocate( nLength );
		input.readHexLine( s.buffer(), sizeof(wchar) * nLength );
	}

	return input;
}

inline const OutStream & operator<<( const OutStream & output, const WideString & s )
{
	if ( (output.filter() & FF_TEXT) == 0 )												
	{
		int nLength = s.length() | 0x80000000;
		output.write( &nLength, sizeof(int) );
		output.write( s.buffer(), sizeof(wchar) * (nLength & 0x7FFFFFFF) );
	}
	else
	{
		int nLength = s.length();
		output << nLength;
		output.writeHexLine( s.buffer(), sizeof(wchar) * nLength );
	}

	return output;
}

//---------------------------------------------------------------------------------------------------

inline const OutStream& operator<<( const OutStream & output, const char * pString )
{
	if ( (output.filter() & FF_TEXT) == 0 )
		output.write( pString, (int)strlen( pString ) * sizeof(char) );
	else
		output.writeLine( pString, true );
	return output;
}

inline const OutStream& operator<<( const OutStream & output, const wchar * pString )
{
	if ( (output.filter() & FF_TEXT) == 0 )
		output.write( pString, (int)(strlen( pString ) * sizeof(wchar)) );
	else
		output.writeHexLine( pString, (int)(strlen( pString ) * sizeof(wchar)) );
	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, Size<T> & size )
{
	if ( (input.filter() & FF_TEXT) == 0 )
		input.read( &size, sizeof(size) );
	else
		input.readHexLine( &size, sizeof(size) );
	return input;
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const Size<T> &size )
{
	if ( (output.filter() & FF_TEXT) == 0 )
		output.write( &size, sizeof(size));
	else
		output.writeHexLine( &size, sizeof(size) );
	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, Point<T> & point )
{
	if ( (input.filter() & FF_TEXT) == 0 )
		input.read( &point, sizeof(point) );
	else
		input.readHexLine( &point, sizeof(point) );
	return input;
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const Point<T> &point )
{
	if ( (output.filter() & FF_TEXT) == 0 )
		output.write( &point, sizeof(point));
	else
		output.writeHexLine( &point, sizeof(point) );
	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, Rect<T> & rect )
{
	if ( (input.filter() & FF_TEXT) == 0 )
		input.read( &rect, sizeof(rect) );
	else
		input.readHexLine( &rect, sizeof(rect) );
	return input;
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const Rect<T> &rect )
{
	if ( (output.filter() & FF_TEXT) == 0 )
		output.write( &rect, sizeof(rect));	
	else
		output.writeHexLine( &rect, sizeof(rect) );
	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, Array<T> &array )
{
	int count;
	input >> count;

	array.allocate( count );
	for(int i=0;i<count;++i)
		input >> array[i];
	return input;
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const Array<T> &array )
{
	output << array.size();
	for(int i=0;i<array.size();++i)
		output << array[i];
	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, std::vector<T> & v )
{
	int count;
	input >> count;

	v.resize( count );
	for(int i=0;i<count;++i)
		input >> v[i];
	return input;
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const std::vector<T> & v )
{
	output << v.size();
	for(size_t i=0;i<v.size();++i)
		output << v[i];
	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, SortedArray<T> &array )
{
	int count;
	input >> count;

	for(int i=0;i<count;++i)
	{
		T object;
		input >> object;
		
		array.insert( object );
	}

	return input;
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const SortedArray<T> &array )
{
	output << array.size();
	for(int i=0;i<array.size();++i)
		output << array[i];
	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, Reference<T> & ref )
{
	T *	pointer = ref;
	input >> pointer;
	
	ref = pointer;

	return input;
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const Reference<T> &ref )
{
	output << ref.pointer();
	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, List<T> & li )
{
	int count;
	input >> count;

	typename List<T>::Iterator listItem;
	for(int i=0;i<count;++i)
	{
		T object;
		input >> object;

		listItem = li.insertAfter( listItem, object );
	}
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const List<T> & li )
{
	output << li.size();

	typename List<T>::Iterator i = li.head();
	for(;i.valid();++i)
		output << *i;

	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, std::list<T> & li )
{
	int count;
	input >> count;

	for(int i=0;i<count;++i)
	{
		T object;
		input >> object;

		li.push_back( object );
	}
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const std::list<T> & li )
{
	output << li.size();
	for( typename std::list<T>::const_iterator i = li.begin(); i != li.end(); ++i )
		output << *i;

	return output;
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const InStream & operator>>( const InStream & input, Queue<T> &queue )
{
	int count;
	input >> count;

	for(int i=0;i<count;++i)
	{
		T object;
		input >> object;

		queue.push( object );
	}
	return input;
}

template<class T>
inline const OutStream & operator<<( const OutStream & output, const Queue<T> &queue )
{
	return( operator<<( output, queue.list() ) );
}

//---------------------------------------------------------------------------------------------------

template<class K,class T>
inline const InStream & operator>>( const InStream & input, Tree<K,T> &tree )
{
	int count;
	input >> count;

	for(int i=0;i<count;++i)
	{
		K key;
		input >> key;
		T object;
		input >> object;

		tree.insert( key, object );
	}

	return input;
}

template<class K,class T>
inline const OutStream & operator<<( const OutStream & output, const Tree<K,T> &tree )
{
	output << tree.size();

	typename Tree<K,T>::Iterator i = tree.head();
	for(;i.valid();++i)
		output << i.key() << *i;

	return output;
}

//---------------------------------------------------------------------------------------------------

template<class K,class T>
inline const InStream & operator>>( const InStream & input, std::map<K,T> &map )
{
	int count;
	input >> count;

	for(int i=0;i<count;++i)
	{
		K key;
		input >> key;
		T object;
		input >> object;

		map.insert( std::pair<K,T>( key, object ) );
	}

	return input;
}

template<class K,class T>
inline const OutStream & operator<<( const OutStream & output, const std::map<K,T> & map )
{
	output << map.size();
	for( typename std::map<K,T>::const_iterator i = map.begin(); i != map.end(); ++i )
		output << i->first << i->second;

	return output;
}

//---------------------------------------------------------------------------------------------------

template<class K,class T,unsigned int S>
inline const InStream & operator>>( const InStream & input, Hash<K,T,S> &hash )
{
	int nCount;
	input >> nCount;

	for(int i=0;i<nCount;++i)
	{
		K key;
		input >> key;
		T object;
		input >> object;

		hash.insert( key, object );
	}

	return input;
}

template<class K,class T,unsigned int S>
inline const OutStream & operator<<( const OutStream & output, const Hash<K,T,S> &hash )
{
	output << hash.size();
	for( typename Hash<K,T,S>::Iterator i = hash.head(); i.valid(); ++i)
		output << i.key() << *i;
	return output;
}

//---------------------------------------------------------------------------------------------------

template<class K,class T,unsigned int S>
inline const InStream & operator>>( const InStream & input, HashArray<K,T,S> & hashArray )
{
	int nCount;
	input >> nCount;

	for(int i=0;i<nCount;++i)
	{
		K key;
		input >> key;
		T object;
		input >> object;

		hashArray.insert( key, object );
	}

	return input;
}

template<class K,class T, unsigned int S>
inline const OutStream & operator<<( const OutStream & output, const HashArray<K,T,S> & hashArray )
{
	output << hashArray.size();
	for(int i=0;i<hashArray.size();++i)
		output << hashArray.key( i ) << hashArray[ i ];
	return output;
}

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, Buffer & buffer )
{
	dword nBytes;
	input >> nBytes;

	void * pBuffer = new byte[ nBytes ];
	if ( (input.filter() & FF_TEXT) == 0 )
		input.read( pBuffer, nBytes );
	else
		input.readHexLine( pBuffer, nBytes );

	buffer.set( pBuffer, nBytes );
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const Buffer & buffer)
{
	output << buffer.bufferSize();
	if ( (output.filter() & FF_TEXT) == 0 )
		output.write( buffer.buffer(), buffer.bufferSize() );
	else
		output.writeHexLine( buffer.buffer(), buffer.bufferSize() );
	return output;
}

//---------------------------------------------------------------------------------------------------

template<typename T>
inline const InStream & operator>>( const InStream & input, Dictionary::TypedStorage<T> * a_pStorage )
{
	if ( a_pStorage == NULL )
		throw File::FileError();
	
	// remove our dictionary from the stream, so we don't try to use a dictionary to stream a dictionary.
	Dictionary::Ref pSaveDictionary = input.dictionary();
	const_cast<InStream &>( input ).setDictionary( NULL );

	input >> a_pStorage->m_nNextID;
	input >> a_pStorage->m_IDMap;
	a_pStorage->initDataMap();

	// restore our dictionary..
	const_cast<InStream &>( input ).setDictionary( pSaveDictionary );

	return input;
}

template<typename T>
inline const OutStream & operator<<( const OutStream & output, const Dictionary::TypedStorage<T> * a_pStorage )
{
	if ( a_pStorage != NULL )
	{
		// remove our dictionary from the stream, so we don't try to use a dictionary to stream a dictionary.
		Dictionary::Ref pSaveDictionary = output.dictionary();
		const_cast<OutStream &>( output ).setDictionary( NULL );

		output << a_pStorage->m_nNextID;
		output << a_pStorage->m_IDMap;

		// restore our dictionary..
		const_cast<OutStream &>( output ).setDictionary( pSaveDictionary );
	}
	else
	{
		output << (u32)0 << (size_t)0;
	}

	return output;
}

//---------------------------------------------------------------------------------------------------


#endif

//--------------------------------------------------------------------------
// EOF
