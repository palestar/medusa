; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
ClassCount=2
ResourceCount=2
NewFileInclude1=#include "stdafx.h"
Class1=CChildFrame
LastClass=CPortView
LastTemplate=CFormView
Resource1=IDR_PORT
Class2=CPortView
Resource2=IDD_FORMVIEW

[CLS:CChildFrame]
Type=0
HeaderFile=ChildFrame.h
ImplementationFile=ChildFrame.cpp
BaseClass=CMDIChildWnd
Filter=M
LastObject=IDC_LIST1
VirtualFilter=mfWC

[MNU:IDR_PORT]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_SEND_MAIL
Command10=ID_FILE_MRU_FILE1
Command11=ID_APP_EXIT
Command12=ID_EDIT_UNDO
Command13=ID_EDIT_CUT
Command14=ID_EDIT_COPY
Command15=ID_EDIT_PASTE
Command16=ID_EDIT_RENAME
Command17=ID_VIEW_TOOLBAR
Command18=ID_VIEW_STATUS_BAR
Command19=ID_VIEW_OPTIONS
Command20=ID_VIEW_REFRESH
Command21=ID_VIEW_CLASSHEIRARCHY
Command22=ID_WINDOW_NEW
Command23=ID_WINDOW_CASCADE
Command24=ID_WINDOW_TILE_HORZ
Command25=ID_WINDOW_ARRANGE
Command26=ID_APP_ABOUT
CommandCount=26

[DLG:IDD_FORMVIEW]
Type=1
Class=CPortView
ControlCount=6
Control1=IDC_LIST1,SysListView32,1350631429
Control2=IDC_BUTTON1,button,1342242816
Control3=IDC_BUTTON2,button,1342242816
Control4=IDC_BUTTON3,button,1342242816
Control5=IDC_EDIT1,edit,1350633600
Control6=IDC_BUTTON4,button,1342242816

[CLS:CPortView]
Type=0
HeaderFile=PortView.h
ImplementationFile=PortView.cpp
BaseClass=CFormView
Filter=D
VirtualFilter=VWC
LastObject=IDC_BUTTON4

