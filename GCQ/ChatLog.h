// ChatLog.h: interface for the ChatLog class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_CHATLOG_H__11D8B841_6D82_11D6_9410_00001CDB2E9A__INCLUDED_)
#define AFX_CHATLOG_H__11D8B841_6D82_11D6_9410_00001CDB2E9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define REL_LOGPATH "ChatLogs\\"

class ChatLog  
{
public:
	void write( CString sLogText );
	bool isLogging();
	void stopLogging();
	bool startLogging();
	ChatLog();
	ChatLog( CString nickName );
	virtual ~ChatLog();

private:
	bool openLogFile();
	CString sNickName;
	CString getAppPath();
	void checkDirectory();
	CString sCurrFileName;
	CStdioFile *fileStream;
	bool bIsActive;
	void writeSeperator();
	void writeHTMLheader();
	CString generateFileName();
	void finishHTMLfile();
};

#endif // !defined(AFX_CHATLOG_H__11D8B841_6D82_11D6_9410_00001CDB2E9A__INCLUDED_)
