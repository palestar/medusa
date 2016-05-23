/*
	NodeBooleanTime.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_BOOLEAN_TIME_H
#define NODE_BOOLEAN_TIME_H

#include "Render3D/BaseNode.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeBooleanTime : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeBooleanTime>			Ref;

	// Construction
	NodeBooleanTime();

	// Node Interface
	void				preRender( RenderContext &context, 
							const Matrix33 & frame, const Vector3 & position );
	// Accessors
	float				begin() const;
	float				end() const;
	bool				invert() const;
	bool				warpTime() const;
	bool				loop() const;
	float				loopEnd() const;

	// Mutators
	void				setTime( float begin, float end, bool invert, bool warpTime );
	void				setLoop( bool loop, float loopEnd );

private:
	// Data
	float				m_Begin, m_End;
	bool				m_Invert;
	bool				m_WarpTime;
	bool				m_Loop;
	float				m_LoopEnd;
};

//----------------------------------------------------------------------------

inline float NodeBooleanTime::begin() const
{
	return( m_Begin );
}

inline float NodeBooleanTime::end() const
{
	return( m_End );
}

inline bool NodeBooleanTime::invert() const
{
	return( m_Invert );
}

inline bool NodeBooleanTime::warpTime() const
{
	return( m_WarpTime );
}

inline bool NodeBooleanTime::loop() const
{
	return( m_Loop );
}

inline float NodeBooleanTime::loopEnd() const
{
	return( m_LoopEnd );
}

inline void NodeBooleanTime::setTime( float begin, float end, bool invert, bool warpTime )
{
	m_Begin = begin;
	m_End = end;
	m_Invert = invert;
	m_WarpTime = warpTime;
}

inline void NodeBooleanTime::setLoop( bool loop, float loopEnd )
{
	m_Loop = loop;
	m_LoopEnd = loopEnd;
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF
