; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CPCSCSampleDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "PCSCSample.h"

ClassCount=4
Class1=CPCSCSampleApp
Class2=CPCSCSampleDlg
Class3=CAboutDlg

ResourceCount=6
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_PCSCSAMPLE_DIALOG
Resource4=IDD_PCSCSAMPLE_DIALOG (English (U.S.))
Resource5=IDD_ABOUTBOX (English (U.S.))
Class4=CReaderDlg
Resource6=IDD_READERS_DIALOG (English (U.S.))

[CLS:CPCSCSampleApp]
Type=0
HeaderFile=PCSCSample.h
ImplementationFile=PCSCSample.cpp
Filter=N
LastObject=CPCSCSampleApp

[CLS:CAboutDlg]
Type=0
HeaderFile=PCSCSampleDlg.h
ImplementationFile=PCSCSampleDlg.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Class=CAboutDlg


[DLG:IDD_PCSCSAMPLE_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CPCSCSampleDlg

[DLG:IDD_PCSCSAMPLE_DIALOG (English (U.S.))]
Type=1
Class=CPCSCSampleDlg
ControlCount=13
Control1=IDC_CONNECT_SC,button,1342242816
Control2=IDC_CARD_DISCONNECT,button,1476460544
Control3=IDC_TRANSMIT_SC,button,1476460544
Control4=IDC_STATIC,button,1342177287
Control5=IDC_STATIC,button,1342177287
Control6=IDC_COMBO_IN,combobox,1344340034
Control7=IDC_EDIT_OUT,edit,1352730628
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC_READER,static,1342308352
Control11=IDC_STATIC,static,1342177283
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_READERS_DIALOG (English (U.S.))]
Type=1
Class=CReaderDlg
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_READERS,combobox,1344340226

[CLS:CReaderDlg]
Type=0
HeaderFile=ReaderDlg.h
ImplementationFile=ReaderDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CReaderDlg
VirtualFilter=dWC

[CLS:CPCSCSampleDlg]
Type=0
HeaderFile=pcscsampledlg.h
ImplementationFile=pcscsampledlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_WRITEDATA

