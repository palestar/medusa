// ChatLog.cpp: implementation of the ChatLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChatLog.h"

#include "MetaClient.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ChatLog::ChatLog( CString nickName )
{
	sNickName = nickName;
}


ChatLog::~ChatLog()
{
	this->stopLogging();
}


bool ChatLog::startLogging()
{
	if ( bIsActive )	// ignore if already logging
		return false;
	
	this->checkDirectory();	// make sure logfile directory exists
	
	if ( this->openLogFile() )		// logfile already existed ?
		this->writeSeperator();		// yes, then write seperator
	else
		this->writeHTMLheader();	// no, so write HTML header
		
	if( fileStream )		// file opened successfull ?
	{
		bIsActive = true;	// ok, then start logging
		return true;	
	}
	else
		return false;		// report error
}


void ChatLog::stopLogging()
{
	if ( !bIsActive )	// ignore if not logging
		return;
	
	bIsActive = false;
	fileStream->Close();
}


void ChatLog::finishHTMLfile()
{
	if ( !bIsActive )	// ignore if not logging
		return;
	
	this->write("</body></html>");
}


CString ChatLog::generateFileName()
{
	CTime tCurrTime = CTime::GetCurrentTime();
	
	return String( "GCQL_%s_%s.html", sNickName, tCurrTime.Format("%Y.%m.%d") );
}


bool ChatLog::isLogging()
{
	return bIsActive;
}


void ChatLog::writeHTMLheader()
{
	if ( !fileStream )
		return;
	
	CTime tCurrTime = CTime::GetCurrentTime();

	this->write("<HTML>\n  <HEAD>\n   <TITLE>GameCQ Chatlog by "+ sNickName 
			+ tCurrTime.Format(" %m/%d/%Y") + "</TITLE>\n  </HEAD>\n"
			+ "   <BODY bgcolor=\"#000000\" text=\"#FFFFFF\">");

}


void ChatLog::writeSeperator()
{
	if ( !fileStream )
		return;

	CTime tCurrTime = CTime::GetCurrentTime();

	this->write("<hr><h3><font color=\"#FFFFFF\">Logging resumed at " + tCurrTime.Format("%m/%d/%Y")
			+ "</font></h3><hr>");
}


void ChatLog::write(CString sLogText)
{
	if ( this->generateFileName().Compare( sCurrFileName ) != 0 )	// got above the 24h barrier ?
	{
		this->finishHTMLfile();	// so close the old file
		this->stopLogging();
		this->startLogging();	// and create a new one for the new date
	}
	
	fileStream->WriteString( sLogText );
}


void ChatLog::checkDirectory()
{
	HANDLE			fFile;			// File Handle
	WIN32_FIND_DATA	fileinfo;		// File Information Structure
	CString			logFilePath = this->getAppPath() + REL_LOGPATH;
	fFile = FindFirstFile( logFilePath ,&fileinfo);

	// if the file exists and it is a directory
	if( fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
		FindClose( fFile );	//  Directory Exists close file
	else
		if( CreateDirectory( logFilePath ,NULL ) )
			SetFileAttributes( logFilePath ,FILE_ATTRIBUTE_NORMAL );
		
}


CString ChatLog::getAppPath()
{
	char szBuffer[_MAX_PATH]; 

	VERIFY(::GetModuleFileName( AfxGetInstanceHandle(), szBuffer, _MAX_PATH) );

	CString sPath = (CString)szBuffer;
	sPath = sPath.Left( sPath.ReverseFind('\\') );

	return sPath;
}


bool ChatLog::openLogFile()
{
	bool			result = false;
	HANDLE			fFile;			// File Handle
	WIN32_FIND_DATA	fileinfo;		// File Information Structure
	CString			logFilePath = this->getAppPath() + REL_LOGPATH + "\\" + sCurrFileName;
	fFile = FindFirstFile( logFilePath ,&fileinfo);

	if( fileinfo.dwFileAttributes == FILE_ATTRIBUTE_NORMAL )
	{
		FindClose( fFile );	//  File exists
		result = true;
	}

	sCurrFileName = this->generateFileName();
	fileStream = new CStdioFile( logFilePath , CFile::modeNoTruncate 
						| CFile::modeWrite | CFile::shareDenyWrite | CFile::typeText );
	
	return result;
}
