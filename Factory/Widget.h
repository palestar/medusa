/*
	Widget.h

	Widget class, created by a factory class
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WIDGET_H
#define WIDGET_H

#include "Standard/Hash.h"
#include "Standard/WidgetKey.h"
#include "Standard/CriticalSection.h"
#include "Standard/Reference.h"
#include "Standard/WeakReference.h"
#include "Reflection/PropertyClass.h"
#include "Standard/Exception.h"
#include "File/Stream.h"
#include "Debug/Error.h"

#include "ClassKey.h"
#include "WidgetKey.h"

#include <map>
#include <vector>

#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

// forward declarations
class Factory;

//----------------------------------------------------------------------------

#define STRINGFY( CLASS )			(#CLASS)
#define STRINGFY2( PARENT, CLASS )	(#PARENT "::" #CLASS )

//----------------------------------------------------------------------------

#define DECLARE_WIDGET_CLASS()																				\
	virtual Factory *			factory() const;															\
	static Factory *			staticFactory();															\
	static const ClassKey &		classKey();

// helpful macro, you must be linked to the DLL or library that contains the class
#define CLASS_KEY( CLASS )		( CLASS::classKey() )

//----------------------------------------------------------------------------

#pragma warning(disable:4251)	// d:\projects\hydra\factory\widget.h(54) : warning C4251: 's_WidgetHash' : class 'Hash<class WidgetKey,class Widget *,32>' needs to have dll-interface to be used by clients of class 'Widget'
#pragma warning(disable:4275)	// Warning C4275: non dll-interface class 'PropertyClass' used as base for dll-interface class 'Widget'

class DLL Widget : public PropertyClass
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();
		
	enum {
		WIDGET_HASH_BITS = 10,				// (1 << 10) = 1024
		WIDGET_HASH_SIZE = 1 << WIDGET_HASH_BITS,
		WIDGET_HASH_MASK = WIDGET_HASH_SIZE - 1
	};

	// Types
	typedef Reference< Widget >							Ref;
	typedef std::map< qword, Widget * >					WidgetMap;

	//! This object allows the WidgetReference<> class to have a pointer to a Widget object which can be auto-updated.
	struct WidgetHandle : public Referenced
	{
		typedef Reference< WidgetHandle >		Ref;
		typedef WeakReference< WidgetHandle >	WeakRef;

		WidgetHandle( Widget::Ref a_pWidget ) : m_pWidget( a_pWidget )
		{}
		~WidgetHandle()
		{
			if ( m_pWidget.valid() )
				m_pWidget->m_pWidgetHandle = NULL;
		}

		Widget::Ref				m_pWidget;
	};

	// Static Data
	static bool					sm_bEnableTracking;						// set to true to enable tracking of all widget objects

	// Exceptions
	DECLARE_EXCEPTION( BadWidget );
	DECLARE_EXCEPTION( KeyConflict );

	// Construction
	Widget();
	virtual	~Widget();

	// Accessors
	const WidgetKey &			key( bool a_bGenKey = true ) const;		// this widgets unique id, if a_bGeKey is true a key will be generated if the ID is NULL_WIDGET (0)
	dword						version() const;						// this widgets version number

	// Mutators
	virtual	bool				write( const OutStream & output );		
	virtual bool				read( const InStream & input );			

	void						setVersion( dword a_nVersion );			// set the version number
	void						updateVersion();						// increments the version by 1

	void						generateKey();							// generate a new key for this Widget
	virtual void				setKey( const WidgetKey &key );			// change the key
	virtual Widget *			copy();									// creates a unique copy of this object
	virtual Widget *			clone();								// make a exact copy of this object

	WidgetHandle::Ref			grabWidgetHandle();						// get/create a reference object for this widget

	// Helpers
	bool						isType( const ClassKey & nClass ) const;	// is the widget of the class type

	// Static
	static WidgetMap &			widgetMap( const WidgetKey & key );		// hash of all widget objects
	static CriticalSection &	widgetLock();							// lock for accessing widget hash
	static Widget::Ref			findWidget( const WidgetKey & key );	// locate a widget by it's key, caller must call releaseReference() when done...
	static Widget::Ref			findWidgetByType( const WidgetKey & key, 
									const ClassKey & ck );
	static CharString			dumpWidgets();							// return all allocated widgets in a string
	static void					dumpWidgetsTrace();						// dump all allocated widgets to trace output

protected:
	void						registerKey();
	void						unregisterKey();

	// Data
	dword						m_nVersion;								// version of this object
	WidgetKey					m_Key;									// unique ID of this object
	WidgetHandle::WeakRef		m_pWidgetHandle;						// weak reference to our WidgetHandle
};

//----------------------------------------------------------------------------

inline const WidgetKey & Widget::key( bool a_bGenKey /*= true*/ ) const
{
	// lazy key creation, only make the key when a key is requested for a Widget
	if ( m_Key.m_Id == 0 && a_bGenKey )
		const_cast<Widget *>( this )->generateKey();
	return m_Key;
}

inline dword Widget::version() const
{
	return m_nVersion;
}

//---------------------------------------------------------------------------------------------------

inline Widget::WidgetHandle::Ref Widget::grabWidgetHandle() 
{
	AutoLock lock( &widgetLock() );		// take a lock, so a Widget doesn't end up with two WidgetHandle objects!
	if ( m_Key.m_Id == 0 )
		generateKey();
	if (! m_pWidgetHandle.valid() )
		m_pWidgetHandle = new WidgetHandle( this );

	return m_pWidgetHandle.pointer();
}

inline void Widget::setVersion( dword a_nVersion )
{
	m_nVersion = a_nVersion;
}

inline void Widget::updateVersion()
{
	++m_nVersion;
}

inline void Widget::generateKey()
{
	AutoLock lock( &widgetLock() );		// take a lock, so a Widget doesn't end up with two WidgetHandle objects!
	if ( m_Key.m_Id != 0 )
		unregisterKey();
	m_Key.requestNumber();
	registerKey();
}

//----------------------------------------------------------------------------

// The following template function should cast a Widget pointer to the specified type of pointer
// This function should be used instead of "dynamic_cast"
template<class T>
inline T * WidgetCast( Widget * pWidget )
{
	T * pCast = NULL;
	if ( pWidget != NULL && pWidget->isType( T::classKey() ) )
		pCast = (T *)pWidget;		// widget is the correct type, explicitly cast

	return pCast;
}

template<class T>
inline const T * WidgetCast( const Widget * pWidget )
{
	T * pCast = NULL;
	if ( pWidget != NULL && pWidget->isType( T::classKey() ) )
		pCast = (T *)pWidget;		// widget is the correct type, explicitly cast

	return pCast;
}

//----------------------------------------------------------------------------

#endif

//--------------------------------------------------------------------------------------
// EOF
