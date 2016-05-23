/*
	Port.h

	Base class for all port classes. 
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PORT_H
#define PORT_H

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include "Factory/Widget.h"
#include "Standard/Referenced.h"
#include "Standard/Library.h"
#include "Resource/Resource.h"

#include "Tools/ResourcerDoc/ResourcerDll.h"

//----------------------------------------------------------------------------

class CResourcerDoc;		// forward declare

//----------------------------------------------------------------------------

#define MAX_RESOURCE_PORTS					(0x100)
#define ID_RESOURCE_PORT_BEGIN				(63000)
#define ID_RESOURCE_PORT_END				(ID_RESOURCE_PORT_BEGIN + MAX_RESOURCE_PORTS)

class DLL Port : public CDocument, public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< Port >				Ref;
	typedef Array< CharString >				DependentArray;
	typedef Array< CharString * >			StringArray;

	typedef Resource						Resource;
	typedef InStream						InStream;
	typedef OutStream						OutStream;
	typedef ClassKey						ClassKey;
	typedef WidgetKey						WidgetKey;
	typedef CharString						CharString;
	typedef Factory							Factory;
	typedef File							File;

	struct PortInfo 
	{
		ClassKey			classKey;
		WidgetKey			widgetKey;
		dword				version;

		int					type;
		HICON				icon;
		bool				locked;
	};

	// Construction
	Port();
	virtual ~Port();

	// CDocument interface
	virtual void			OnChangedViewList();
	// Widget interface
	bool					read( const InStream & );
	// Client Interface
	virtual bool			autoLoad();										// should resource be autoloaded, default is false
	virtual void			dependencies( DependentArray & dep );			// get the depdendencies
	virtual CFrameWnd *		createView();									// create a view of the port
	virtual	Resource::Ref	createResource() = 0;							// create the resource

	virtual bool			importFile( const char * fileName ) = 0;		// import from file
	virtual void			shellOpen() const = 0;							// open the source object through a shell program

	// Accessors
	bool					locked() const;									// is this document currently locked
	// Mutators
	void					setFileLock( File * pFile );
	void					updateKey();		
	// Helpers
	void					report( const char * pMessage );				// report error or message

	// Static
	static CResourcerDoc *	document();										// get the current resourcer document
	static CharString		relative( const char * pFile );				// get a filename/path relative to our current document

	static void				loadPortLibs();									// called to load all the DLL's
	static void				unloadPortLibs();								// unloads the port libraries

	static void				addDocTemplate();								// add this document type to the current CWinApp

	static void				registerPort( const ClassKey &key, HICON icon );
	static void				registerImportExt( const ClassKey &key, const char * extension );	
	static void				unregisterPort( const ClassKey &key );
	static void				unregisterImportExt( const char * extension );

	static int				portCount();									// number of port types available
	static ClassKey			portKey( int n );
	static Factory *		portFactory( int n );
	static CString			portName( int n );
	static HICON			portIcon( int n );
	static int				portIndex( Port * pPort );
	static int				portIndex( const ClassKey & key );

	static bool				portInfo( const char * file, PortInfo & info );
	static Port::Ref		portLoad( const char * file, bool lock = false );
	static Resource::Link	portResource( const char * file );
	static void				portView( const char * file );

	static int				findImportExt( const char * extension );
	static dword			getFileTime( const char * file );

	static void				pushMessage( const char * pMessage );

private:
	// Data
	File::Ref				m_pFileLock;	
	bool					m_bViewActive;

	// Static
	static Array< ClassKey >
							s_Ports;
	static Hash< ClassKey, HICON >
							s_PortIcon;
	static Tree< CString, Array< ClassKey > >
							s_PortImportExt;

	static Array<CString> 
							s_PortLibPaths;					// the array of all the port DLL's
	static Array< Library * >
							s_PortLibs;						// array of loaded DLL's

	friend class CChoosePorts;
	friend class CSelectImportPort;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Port)
	public:
	virtual BOOL OnOpenDocument(const char * lpszPathName);
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

	// Generated message map functions
public:
	//{{AFX_MSG(Port)
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//----------------------------------------------------------------------------

class DLL PortTemplate : public CMultiDocTemplate
{
public:
	// Construction
	PortTemplate( UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass ) : 
			CMultiDocTemplate( nIDResource, pDocClass, pFrameClass, pViewClass )
	{}
	virtual ~PortTemplate();

	// CDocTemplate interface
	virtual CDocument* CreateNewDocument();
	virtual CDocument* OpenDocumentFile( LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE );
	virtual CFrameWnd* CreateNewFrame( CDocument* pDoc, CFrameWnd* pOther );
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif

//----------------------------------------------------------------------------
// EOF

