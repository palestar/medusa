; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Resourcer.h"
LastPage=0

ClassCount=6
Class1=CResourcerApp
Class2=CClassHeirarchy
Class3=CUpdateSettings
Class4=CMainFrame

ResourceCount=5
Resource1=IDD_CLASS_HEIRARCHY
Resource2=IDD_UPDATE_SETTINGS
Class5=CClientUpdate
Class6=CAboutDlg
Resource3=IDD_ABOUTBOX
Resource4=IDD_CLIENT_UPDATE
Resource5=IDR_MAINFRAME

[CLS:CResourcerApp]
Type=0
HeaderFile=Resourcer.h
ImplementationFile=Resourcer.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC
LastObject=ID_APP_ABOUT

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=ID_EDIT_SELECT_ALL
BaseClass=CMDIFrameWnd
VirtualFilter=fWC


[CLS:CAboutDlg]
Type=0
HeaderFile=Resourcer.cpp
ImplementationFile=Resourcer.cpp
Filter=D
LastObject=ID_VIEW_PORTLIBRARIES

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_VIEW_CLASSHEIRARCHY
Command9=ID_VIEW_UPDATESETTINGS
Command10=ID_VIEW_PORTLIBRARIES
Command11=ID_APP_ABOUT
CommandCount=11

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_EDIT_SELECT_ALL
Command2=ID_EDIT_COPY
Command3=ID_FILE_NEW
Command4=ID_FILE_OPEN
Command5=ID_FILE_PRINT
Command6=ID_FILE_SAVE
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_NEXT_PANE
Command11=ID_PREV_PANE
Command12=ID_EDIT_COPY
Command13=ID_EDIT_PASTE
Command14=ID_EDIT_CUT
Command15=ID_EDIT_UNDO
CommandCount=15

[DLG:IDD_CLASS_HEIRARCHY]
Type=1
Class=CClassHeirarchy
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDC_TREE1,SysTreeView32,1350631431

[CLS:CClassHeirarchy]
Type=0
HeaderFile=ClassHeirarchy.h
ImplementationFile=ClassHeirarchy.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_TREE1

[DLG:]
Type=1
Class=?
ControlCount=5
Control1=IDCANCEL,button,1342242816
Control2=IDC_PROGRESS1,msctls_progress32,1350565888
Control3=IDC_PROGRESS2,msctls_progress32,1350565888
Control4=IDC_FILESTATUS,static,1342308352
Control5=IDC_STATUS,static,1342308352

[CLS:CClientUpdate]
Type=0
HeaderFile=ClientUpdate.h
ImplementationFile=ClientUpdate.cpp
BaseClass=CDialog

[DLG:IDD_CLIENT_UPDATE]
Type=1
Class=?
ControlCount=5
Control1=IDCANCEL,button,1342242816
Control2=IDC_PROGRESS1,msctls_progress32,1350565888
Control3=IDC_PROGRESS2,msctls_progress32,1350565888
Control4=IDC_FILESTATUS,static,1342308352
Control5=IDC_STATUS,static,1342308352

[DLG:IDD_UPDATE_SETTINGS]
Type=1
Class=CUpdateSettings
ControlCount=11
Control1=IDC_CHECK1,button,1342242819
Control2=IDC_EDIT1,edit,1350631552
Control3=IDC_EDIT4,edit,1350631584
Control4=IDC_EDIT2,edit,1350631552
Control5=IDC_EDIT3,edit,1350631552
Control6=IDOK,button,1342242817
Control7=IDCANCEL,button,1342242816
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352

[CLS:CUpdateSettings]
Type=0
HeaderFile=UpdateSettings.h
ImplementationFile=UpdateSettings.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_EDIT1

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_DOCUMENT_OPTIONS
Command9=ID_DOCUMENT_BUILD
Command10=ID_DOCUMENT_BUILDSELECTED
Command11=ID_DOCUMENT_BUILDALL
Command12=ID_VERSIONCONTROL_CHECKOUT
Command13=ID_VERSIONCONTROL_CHECKIN
Command14=ID_VERSIONCONTROL_UNCHECK
Command15=ID_TREE_CREATE
Command16=ID_TREE_DELETE
Command17=ID_PORTS_DELETE
Command18=ID_PORTS_PROPERTIES
Command19=ID_PORTS_SHELLOPEN
Command20=ID_APP_ABOUT
CommandCount=20

