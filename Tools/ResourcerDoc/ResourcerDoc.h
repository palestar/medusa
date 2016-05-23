// ResourcerDoc.h : interface of the CResourcerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESOURCERDOC_H__F7307290_6F06_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_RESOURCERDOC_H__F7307290_6F06_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "Factory/BrokerArchive.h"
#include "File/FileDisk.h"
#include "Factory/BrokerFolder.h"
#include "Network/MirrorClient.h"
#include "Standard/CharString.h"

#include "Port.h"
#include "ResourcerMirrorClient.h"
#include "ResourcerDll.h"



class DLL CResourcerDoc : public CDocument
{
public:
	// Friends
	friend class DocumentOptions;
	friend class ResourcerList;

	// Accessors
	const char *					rootDirectory() const;						// the root directory
	const char *					brokerFolder() const;						// broker file name
	const char * 					currentDirectory() const;					// the current directory, includes the root directory

	CharString						relative( const char * pFile ) const;		// returns the path to a file relative to the root directory
	CharString						resolve( const char * pFile ) const;		// returns the full path of a file

	int								messageCount() const;
	const char *					message( int n ) const;
	const char *					status() const;

	MirrorClient *					mirror() const;								// get the mirror client object

	// Mutators
	void							setCurrentDirectory( const char * path );

	void							pumpMessages();
	void							beginBuild();
	void							endBuild();

	bool							updateBroker( const char * pMask, bool recurse );
	bool							updateBroker( const char * pFile );
	Resource::Link					getResource( const char * pFile );

	void							upgradePorts( const char * pMask, bool bRecurse );
	void							touchPorts( const char * pMask, bool bRecurse );

	void							lockMessageQueue( bool bLock );
	void							pushMessage( const char * pMessage );
	void							flushMessages();
	void							setStatus( const char * pStatus );			// uses the same lock as the message queue

	WidgetKey						portKey( const char * pFile );				// the key for a port
	CharString						portFile( const WidgetKey & key );			// reverse lookup a port

	bool							checkKeyUnique( const WidgetKey & a_Key );	// checks if checkKeyUnique has been called before with this same key
	void							onPortSaved( const WidgetKey & a_Key );

	// Static
	static void						addDocTemplate();
	static CResourcerDoc *			document();

private:
	// Types
	typedef Port::DependentArray	DependentArray;
	typedef Port::StringArray		StringArray;

	typedef Hash< WidgetKey, DependentArray >	
									DependentTree;
	typedef std::map< WidgetKey, CharString >
									ReverseHash;

	// Data
	bool							m_bBulding;					// are we currently building ports
	time_t							m_nBuildStart;				// time when built started..
	Hash< WidgetKey, dword >		m_Built;					// ports that have been built
	std::set< WidgetKey >			m_Keys;						// keys of all port seen, used to detect conflicts

	CharString						m_RootDirectory;			// location of all common files, a networked directory
	CharString						m_BrokerFolder;				// destination folder
	DependentTree					m_DependentTree;			// port dependencies tree
	ReverseHash						m_ReverseHash;				// 

	CharString						m_MirrorServer;
	int								m_MirrorServerPort;
	CharString						m_UID;
	CharString						m_PW;
	CharString						m_sLocaleFile;

	// begin non-serialized
	CharString						m_CurrentDirectory;			// current selected directory
	BrokerFolder					m_Broker;
	CriticalSection					m_MessageLock;
	Array< CharString >				m_Messages;
	CharString						m_sStatus;

	MirrorClient *					m_pMirrorClient;
	dword							m_nSyncronizeJob;

	static CResourcerDoc *			s_pDocument;

protected: // create from serialization only
	CResourcerDoc();
	DECLARE_DYNCREATE(CResourcerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourcerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(const char * lpszPathName);
	virtual BOOL OnSaveDocument(const char * lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CResourcerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CResourcerDoc)
	afx_msg void OnDocumentOptions();
	afx_msg void OnDocumentBuild();
	afx_msg void OnDocumentClean();
	afx_msg void OnDocumentClearoutput();
	afx_msg void OnDocumentSyncronize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//----------------------------------------------------------------------------

inline int CResourcerDoc::messageCount() const
{
	return m_Messages.size();
}

inline const char * CResourcerDoc::message( int n ) const
{
	return m_Messages[ n ];
}

inline const char *	CResourcerDoc::status() const
{
	return m_sStatus;
}

inline MirrorClient * CResourcerDoc::mirror() const
{
	return m_pMirrorClient;
}

//----------------------------------------------------------------------------

inline void CResourcerDoc::lockMessageQueue( bool bLock )
{
	if ( bLock )
		m_MessageLock.lock();
	else
		m_MessageLock.unlock();
}

//----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCERDOC_H__F7307290_6F06_11D2_B451_00C04F6FF8BD__INCLUDED_)
