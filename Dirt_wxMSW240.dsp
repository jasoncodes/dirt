# Microsoft Developer Studio Project File - Name="Dirt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Dirt - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Dirt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Dirt.mak" CFG="Dirt - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Dirt - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Dirt - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Dirt - Win32 Console Release" (based on "Win32 (x86) Application")
!MESSAGE "Dirt - Win32 Console Debug" (based on "Win32 (x86) Application")
!MESSAGE "Dirt - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "Dirt - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Dirt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MD /W4 /WX /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /YX"wx/wxprec.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG" /d "EXTRA_VER_INFO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib crypto\Release\cryptlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib iphlpapi.lib /nologo /subsystem:windows /machine:I386 /ignore:4089 /ignore:6004
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=echo Packing...	upx --best --no-color --no-progress "Release\Dirt.exe" > nul
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Dirt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MDd /W4 /WX /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR /YX"wx/wxprec.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG" /d "EXTRA_VER_INFO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib crypto/Debug/cryptlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib iphlpapi.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /ignore:4089 /ignore:6004
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Dirt - Win32 Console Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ConsoleRelease"
# PROP BASE Intermediate_Dir "ConsoleRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ConsoleRelease"
# PROP Intermediate_Dir "ConsoleRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MD /W4 /WX /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /YX"wx/wxprec.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG" /d "_CONSOLE" /d "EXTRA_VER_INFO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib crypto\Release\cryptlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib iphlpapi.lib /nologo /entry:"WinMainCRTStartup" /subsystem:console /machine:I386 /ignore:4089 /ignore:6004
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=echo Packing...	upx --best --no-color --no-progress "ConsoleRelease\Dirt.exe" > nul
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Dirt - Win32 Console Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ConsoleDebug"
# PROP BASE Intermediate_Dir "ConsoleDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ConsoleDebug"
# PROP Intermediate_Dir "ConsoleDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MDd /W4 /WX /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR /YX"wx/wxprec.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG" /d "_CONSOLE" /d "EXTRA_VER_INFO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib crypto/Debug/cryptlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib iphlpapi.lib /nologo /entry:"WinMainCRTStartup" /subsystem:console /debug /machine:I386 /pdbtype:sept /ignore:4089 /ignore:6004
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Dirt - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "UnicodeRelease"
# PROP BASE Intermediate_Dir "UnicodeRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "UnicodeRelease"
# PROP Intermediate_Dir "UnicodeRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MD /W4 /WX /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "_UNICODE" /D "UNICODE" /YX"wx/wxprec.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG" /d "_UNICODE" /d "EXTRA_VER_INFO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmswu.lib crypto\Release\cryptlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib iphlpapi.lib /nologo /subsystem:windows /machine:I386 /ignore:4089 /ignore:6004
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=echo Packing...	upx --best --no-color --no-progress "UnicodeRelease\Dirt.exe" > nul
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Dirt - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "UnicodeDebug"
# PROP BASE Intermediate_Dir "UnicodeDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "UnicodeDebug"
# PROP Intermediate_Dir "UnicodeDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MDd /W4 /WX /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "_UNICODE" /D "UNICODE" /FR /YX"wx/wxprec.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG" /d "_UNICODE" /d "EXTRA_VER_INFO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswud.lib crypto/Debug/cryptlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib winmm.lib iphlpapi.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /ignore:4089 /ignore:6004
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Dirt - Win32 Release"
# Name "Dirt - Win32 Debug"
# Name "Dirt - Win32 Console Release"
# Name "Dirt - Win32 Console Debug"
# Name "Dirt - Win32 Unicode Release"
# Name "Dirt - Win32 Unicode Debug"
# Begin Source File

SOURCE=.\BroadcastSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\BroadcastSocket.h
# End Source File
# Begin Source File

SOURCE=.\ByteBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\ByteBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Client.cpp
# End Source File
# Begin Source File

SOURCE=.\Client.h
# End Source File
# Begin Source File

SOURCE=.\ClientDefault.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientDefault.h
# End Source File
# Begin Source File

SOURCE=.\ClientTimers.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientTimers.h
# End Source File
# Begin Source File

SOURCE=.\ClientUIConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientUIConsole.h
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDICanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDICanvas.h
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDIConfigDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDIConfigDialog.h
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDIFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDIFrame.h
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDITransferPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDITransferPanel.h
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDITransferResumeDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDITransferResumeDialog.h
# End Source File
# Begin Source File

SOURCE=.\ConfigFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigFile.h
# End Source File
# Begin Source File

SOURCE=.\Console.cpp
# End Source File
# Begin Source File

SOURCE=.\Console.h
# End Source File
# Begin Source File

SOURCE=.\CPSCalc.cpp
# End Source File
# Begin Source File

SOURCE=.\CPSCalc.h
# End Source File
# Begin Source File

SOURCE=.\Crypt.cpp
# End Source File
# Begin Source File

SOURCE=.\Crypt.h
# End Source File
# Begin Source File

SOURCE=.\CryptSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\CryptSocket.h
# End Source File
# Begin Source File

SOURCE=.\CryptSocketProxy.cpp
# End Source File
# Begin Source File

SOURCE=.\CryptSocketProxy.h
# End Source File
# Begin Source File

SOURCE=.\Dirt.cpp
# End Source File
# Begin Source File

SOURCE=.\Dirt.h
# End Source File
# Begin Source File

SOURCE=.\Dirt.rc
# End Source File
# Begin Source File

SOURCE=.\DNS.cpp
# End Source File
# Begin Source File

SOURCE=.\DNS.h
# End Source File
# Begin Source File

SOURCE=.\File.cpp
# End Source File
# Begin Source File

SOURCE=.\File.h
# End Source File
# Begin Source File

SOURCE=.\FileDropTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\FileDropTarget.h
# End Source File
# Begin Source File

SOURCE=.\FileTransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\FileTransfer.h
# End Source File
# Begin Source File

SOURCE=.\FileTransfers.cpp
# End Source File
# Begin Source File

SOURCE=.\FileTransfers.h
# End Source File
# Begin Source File

SOURCE=.\HTTP.cpp
# End Source File
# Begin Source File

SOURCE=.\HTTP.h
# End Source File
# Begin Source File

SOURCE=.\InputControl.cpp
# End Source File
# Begin Source File

SOURCE=.\InputControl.h
# End Source File
# Begin Source File

SOURCE=.\IPInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\IPInfo.h
# End Source File
# Begin Source File

SOURCE=.\KDE.cpp
# End Source File
# Begin Source File

SOURCE=.\KDE.h
# End Source File
# Begin Source File

SOURCE=.\LanListFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\LanListFrame.h
# End Source File
# Begin Source File

SOURCE=.\Log.h
# End Source File
# Begin Source File

SOURCE=.\LogControl.cpp
# End Source File
# Begin Source File

SOURCE=.\LogControl.h
# End Source File
# Begin Source File

SOURCE=.\LogReader.cpp
# End Source File
# Begin Source File

SOURCE=.\LogReader.h
# End Source File
# Begin Source File

SOURCE=.\LogViewerFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\LogViewerFrame.h
# End Source File
# Begin Source File

SOURCE=.\LogWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\LogWriter.h
# End Source File
# Begin Source File

SOURCE=.\Modifiers.h
# End Source File
# Begin Source File

SOURCE=.\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\Mutex.h
# End Source File
# Begin Source File

SOURCE=.\NickListControl.cpp
# End Source File
# Begin Source File

SOURCE=.\NickListControl.h
# End Source File
# Begin Source File

SOURCE=.\RadioBoxPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\RadioBoxPanel.h
# End Source File
# Begin Source File

SOURCE=.\RCS.cpp
# End Source File
# Begin Source File

SOURCE=.\RCS.h
# End Source File
# Begin Source File

SOURCE=.\Server.cpp
# End Source File
# Begin Source File

SOURCE=.\Server.h
# End Source File
# Begin Source File

SOURCE=.\ServerDefault.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerDefault.h
# End Source File
# Begin Source File

SOURCE=.\ServerUIConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerUIConsole.h
# End Source File
# Begin Source File

SOURCE=.\ServerUIFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerUIFrame.h
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDIPasswordManagerDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientUIMDIPasswordManagerDialog.h
# End Source File
# Begin Source File

SOURCE=.\ServerUIFrameConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerUIFrameConfig.h
# End Source File
# Begin Source File

SOURCE=.\SpanTag.cpp
# End Source File
# Begin Source File

SOURCE=.\SpanTag.h
# End Source File
# Begin Source File

SOURCE=.\Splash.cpp
# End Source File
# Begin Source File

SOURCE=.\Splash.h
# End Source File
# Begin Source File

SOURCE=.\StaticCheckBoxSizer.cpp
# End Source File
# Begin Source File

SOURCE=.\StaticCheckBoxSizer.h
# End Source File
# Begin Source File

SOURCE=.\SwitchBar.cpp
# End Source File
# Begin Source File

SOURCE=.\SwitchBar.h
# End Source File
# Begin Source File

SOURCE=.\SwitchBarCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\SwitchBarCanvas.h
# End Source File
# Begin Source File

SOURCE=.\SwitchBarChild.cpp
# End Source File
# Begin Source File

SOURCE=.\SwitchBarChild.h
# End Source File
# Begin Source File

SOURCE=.\SwitchBarParent.cpp
# End Source File
# Begin Source File

SOURCE=.\SwitchBarParent.h
# End Source File
# Begin Source File

SOURCE=.\TrayIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\TrayIcon.h
# End Source File
# Begin Source File

SOURCE=.\TristateConfigPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\TristateConfigPanel.h
# End Source File
# Begin Source File

SOURCE=.\URL.cpp
# End Source File
# Begin Source File

SOURCE=.\URL.h
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Target
# End Project
