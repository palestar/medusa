; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSettings
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "ProfilerClient.h"

ClassCount=6
Class1=CProfilerClientApp
Class2=CProfilerClientDlg
Class3=CAboutDlg

ResourceCount=6
Resource1=IDD_PROFILERCLIENT_DIALOG
Resource2=IDR_MAINFRAME
Resource3=IDD_SETTINGS
Resource4=IDD_EDIT_SERVER
Resource5=IDD_ABOUTBOX
Class4=CEditAlarm
Class5=CEditServer
Class6=CSettings
Resource6=IDD_EDIT_ALARM

[CLS:CProfilerClientApp]
Type=0
HeaderFile=ProfilerClient.h
ImplementationFile=ProfilerClient.cpp
Filter=N

[CLS:CProfilerClientDlg]
Type=0
HeaderFile=ProfilerClientDlg.h
ImplementationFile=ProfilerClientDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_LIST1

[CLS:CAboutDlg]
Type=0
HeaderFile=ProfilerClientDlg.h
ImplementationFile=ProfilerClientDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_PROFILERCLIENT_DIALOG]
Type=1
Class=CProfilerClientDlg
ControlCount=3
Control1=IDC_STATUS,static,1342308352
Control2=IDC_BUTTON2,button,1342242816
Control3=IDC_TREE1,SysTreeView32,1350635527

[DLG:IDD_SETTINGS]
Type=1
Class=CSettings
ControlCount=15
Control1=IDC_LIST1,SysListView32,1350631425
Control2=IDC_BUTTON1,button,1342242816
Control3=IDC_BUTTON2,button,1342242816
Control4=IDC_BUTTON3,button,1342242816
Control5=IDC_BUTTON8,button,1342242816
Control6=IDC_LIST3,SysListView32,1350631425
Control7=IDC_BUTTON4,button,1342242816
Control8=IDC_BUTTON5,button,1342242816
Control9=IDC_BUTTON6,button,1342242816
Control10=IDC_EDIT1,edit,1350631552
Control11=IDC_BUTTON7,button,1342242816
Control12=IDOK,button,1342242816
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352

[DLG:IDD_EDIT_SERVER]
Type=1
Class=CEditServer
ControlCount=7
Control1=IDC_EDIT1,edit,1350631552
Control2=IDC_EDIT2,edit,1350631552
Control3=IDC_CHECK1,button,1342242819
Control4=IDOK,button,1342242817
Control5=IDCANCEL,button,1342242816
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352

[DLG:IDD_EDIT_ALARM]
Type=1
Class=CEditAlarm
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT1,edit,1350631552
Control5=IDC_CHECK1,button,1342242819
Control6=IDC_STATIC,static,1342308352
Control7=IDC_EDIT2,edit,1350631552

[CLS:CEditAlarm]
Type=0
HeaderFile=EditAlarm.h
ImplementationFile=EditAlarm.cpp
BaseClass=CDialog
Filter=D
LastObject=CEditAlarm
VirtualFilter=dWC

[CLS:CEditServer]
Type=0
HeaderFile=EditServer.h
ImplementationFile=EditServer.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC

[CLS:CSettings]
Type=0
HeaderFile=Settings.h
ImplementationFile=Settings.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_BUTTON8
VirtualFilter=dWC

