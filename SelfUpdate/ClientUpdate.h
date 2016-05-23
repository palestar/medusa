/*
	ClientUpdate.h
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef CLIENT_UPDATE_H
#define CLIENT_UPDATE_H

//----------------------------------------------------------------------------

#include "stdafx.h"
#include "Resource.h"
#include "Debug/SafeThread.h"
#include "Network/MirrorClient.h"

#include "SelfUpdateDll.h"

//----------------------------------------------------------------------------

class DLL ClientUpdate : public CDialog, public MirrorClient
{
// Construction
public:
	// Types
	enum { IDD = IDD_SELF_UPDATE_DIALOG };

	ClientUpdate( const char * mirrorDirectory, 
		const char * mirrorServer, int port, 
		const char * pCatalog = NULL, CWnd* pParent = NULL, dword nSID = 0, bool bSelfUpdating = true );  
	virtual ~ClientUpdate();

	// MirrorClient interface
	virtual void		onStatus( const char * status );
	virtual void		onError( const char * error );
	virtual void		onProgress( dword nTotalBytes, dword nBytes );
	virtual void		onFileProgress( const char * pFile, 
							dword nTotalBytes, dword nBytes );
	virtual bool		onFileDelete( const char * pFile );

	bool				isDone() const;
	bool				isError() const;

	const char *		getStatus() const;

	// Static
	static bool			updateSelf( const char * pSection,			// section in the INI file to find this applications settings
							const char * pAddress = NULL, int nPort = 0,
							const char * pConfig = NULL,			// optional configuration filename, by default config.ini
							const char * pCatalog = NULL );

	// Helpers
	static CString		formatBytes( dword bytes );
	static CString		formatRemainingTime( dword nRemain );

private:

	class UpdateThread : public SafeThread	
	{
	public:
		// Construction
		UpdateThread( ClientUpdate * pClient ) : m_pClient( pClient )
		{}
		// Thread interface
		int run()
		{
			return m_pClient->doUpdate();
		}
	private:
		ClientUpdate *	m_pClient;
	};

	int 				doUpdate();
	UpdateThread *		m_pUpdateThread;
	
	// Dialog Data
	//{{AFX_DATA(ClientUpdate)
	CProgressCtrl	m_FileProgress;
	CProgressCtrl	m_Progress;
	CString	m_FileStatus;
	CString	m_Status;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ClientUpdate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ClientUpdate)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// Data
	CriticalSection		m_Lock;
	CString				m_MirrorDirectory;
	CString				m_MirrorServer;
	int					m_MirrorPort;
	dword				m_nSID;
	bool				m_bSelfUpdating;

	dword				m_nStartTime;
	dword				m_nTotalBytes;
	dword				m_nBytesReceived;

	CString				m_sFileProgress;
	dword				m_nFileSize;
	dword				m_nFileReceived;

	volatile bool		m_bExit;
	volatile bool		m_bError;
};

//---------------------------------------------------------------------------------------------------

inline bool ClientUpdate::isDone() const
{
	return m_bExit;
}

inline bool ClientUpdate::isError() const
{
	return m_bError;
}

inline const char * ClientUpdate::getStatus() const
{
	return m_Status;
}


//---------------------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF
