/*
	Codec.h
	
	Encoder / Decoder abstract interface
	- All Compression routines can be used through this abstract interface
	- A codec is not always a compressor, it may be an encryption or any proces that takes input data and generates output that
	can be decoded by this same object.

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef CODEC_H
#define CODEC_H

#include "Standard/Array.h"
#include "Standard/CharString.h"
#include "MedusaDll.h"

#pragma warning(disable:4275)		// non-dll interface warning for template class

//----------------------------------------------------------------------------

class DLL Codec
{
public:
	// Types
	enum EncodeLevel {
		CL_BEST,			// smallest size, most time
		CL_NORMAL,			// default compression
		CL_FAST,			// fastest compression
		CL_NONE,			// none/faster compression
	};

	class DLL Factory
	{
	public:
		// Construction
		Factory( const char * pName );
		virtual ~Factory();

		const char *			name() const;
		virtual Codec *			create() const = 0;

	private:
		// Data
		CharString				m_sName;
	};
	typedef Array<Factory *>	FactoryArray;

	template<typename T>
	class DLL FactoryTemplate : public Factory
	{
	public:
		FactoryTemplate( const char * pName ) : Factory( pName )
		{}

		// CodecFactory interface
		Codec *	create() const
		{
			return new T();
		}
	};

	//! Create another version of this codec, any internal dictionary will not be copied.
	virtual Codec *			clone() = 0;
	// encode data using this codec, returns the number of bytes put into the output buffer
	virtual int				encode( const void * pInput, int nInputBytes, 
								void * pOutput, int nOutputBytes, EncodeLevel nLevel ) = 0;
	// decode data using this code, returns the number of bytes decoded into the output buffer
	virtual int				decode( const void * pInput, int nInputBytes, 
								void * pOutput, int nOutputBytes ) = 0;
	// reset the codec, this should clear any internal dictionary...
	virtual void			reset() = 0;

	static Codec *			createNamed( const char * pName );
	static FactoryArray &	factoryArray();
	static int				factoryCount();
	static Factory *		factory( int n );
};

//----------------------------------------------------------------------------

//! codec does no encoding, just memcpy's the data..
class NoCodec : public Codec
{
public:
	//! Codec interface
	virtual Codec *			clone();
	virtual int				encode( const void * pInput, int nInputBytes, 
								void * pOutput, int nOutputBytes, EncodeLevel nLevel );
	virtual int				decode( const void * pInput, int nInputBytes, 
								void * pOutput, int nOutputBytes );
	virtual void			reset();
};


#endif

//----------------------------------------------------------------------------
//EOF
