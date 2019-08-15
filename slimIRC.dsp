# Microsoft Developer Studio Project File - Name="slimIRC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=slimIRC - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "slimIRC.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "slimIRC.mak" CFG="slimIRC - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "slimIRC - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "slimIRC - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "slimIRC - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\libirc\\" /I ".\ssl\include\\" /I "." /I ".\lua\\" /FI"pragma.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLE_THREADS" /D "ENABLE_SSL" /D "ENABLE_IPV6" /D _WIN32_IE=0x0500 /FR /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib Ws2_32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=upx .\release\slimIRC.exe	echo program files path=%PROGRAMFILES%	copy .\release\slimIRC.exe "%PROGRAMFILES%\slimIRC\slimIRC.exe"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "slimIRC - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\libirc\\" /I ".\ssl\include\\" /I "." /I ".\lua\\" /FI"pragma.h" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLE_THREADS" /D "ENABLE_SSL" /D "ENABLE_IPV6" /D "ENABLE_DEBUG" /FR /FD /GZ /c
# SUBTRACT CPP /u
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib Ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "slimIRC - Win32 Release"
# Name "slimIRC - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "libircclient"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\libirc\src\libircclient.c
# End Source File
# Begin Source File

SOURCE=.\libirc\src\lua_scripting.c
# End Source File
# Begin Source File

SOURCE=.\libirc\src\portable.c

!IF  "$(CFG)" == "slimIRC - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "slimIRC - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libirc\src\ssl.c

!IF  "$(CFG)" == "slimIRC - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "slimIRC - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "ssl"

# PROP Default_Filter "*.c;*.h"
# Begin Group "library"

# PROP Default_Filter "*.c;*.h"
# Begin Source File

SOURCE=.\ssl\library\aes.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\arc4.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\asn1parse.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\base64.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\bignum.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\camellia.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\certs.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\cipher.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\cipher_wrap.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\ctr_drbg.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\debug.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\des.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\dhm.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\entropy.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\entropy_poll.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\error.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\havege.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md2.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md4.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md5.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md_wrap.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\net.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\padlock.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\pem.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\pkcs11.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\rsa.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\sha1.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\sha2.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\sha4.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\ssl_cli.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\ssl_srv.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\ssl_tls.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\timing.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\version.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\x509parse.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\xtea.c
# End Source File
# End Group
# End Group
# Begin Group "lua"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lua\lapi.c
# End Source File
# Begin Source File

SOURCE=.\lua\lauxlib.c
# End Source File
# Begin Source File

SOURCE=.\lua\lbaselib.c
# End Source File
# Begin Source File

SOURCE=.\lua\lbitlib.c
# End Source File
# Begin Source File

SOURCE=.\lua\lcode.c
# End Source File
# Begin Source File

SOURCE=.\lua\lcorolib.c
# End Source File
# Begin Source File

SOURCE=.\lua\lctype.c
# End Source File
# Begin Source File

SOURCE=.\lua\ldblib.c
# End Source File
# Begin Source File

SOURCE=.\lua\ldebug.c
# End Source File
# Begin Source File

SOURCE=.\lua\ldo.c
# End Source File
# Begin Source File

SOURCE=.\lua\ldump.c
# End Source File
# Begin Source File

SOURCE=.\lua\lfunc.c
# End Source File
# Begin Source File

SOURCE=.\lua\lgc.c
# End Source File
# Begin Source File

SOURCE=.\lua\linit.c
# End Source File
# Begin Source File

SOURCE=.\lua\liolib.c
# End Source File
# Begin Source File

SOURCE=.\lua\llex.c
# End Source File
# Begin Source File

SOURCE=.\lua\lmathlib.c
# End Source File
# Begin Source File

SOURCE=.\lua\lmem.c
# End Source File
# Begin Source File

SOURCE=.\lua\loadlib.c
# End Source File
# Begin Source File

SOURCE=.\lua\lobject.c
# End Source File
# Begin Source File

SOURCE=.\lua\lopcodes.c
# End Source File
# Begin Source File

SOURCE=.\lua\loslib.c
# End Source File
# Begin Source File

SOURCE=.\lua\lparser.c
# End Source File
# Begin Source File

SOURCE=.\lua\lstate.c
# End Source File
# Begin Source File

SOURCE=.\lua\lstring.c
# End Source File
# Begin Source File

SOURCE=.\lua\lstrlib.c
# End Source File
# Begin Source File

SOURCE=.\lua\ltable.c
# End Source File
# Begin Source File

SOURCE=.\lua\ltablib.c
# End Source File
# Begin Source File

SOURCE=.\lua\ltm.c
# End Source File
# Begin Source File

SOURCE=.\lua\lundump.c
# End Source File
# Begin Source File

SOURCE=.\lua\lvm.c
# End Source File
# Begin Source File

SOURCE=.\lua\lzio.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\art_viewer.c
# End Source File
# Begin Source File

SOURCE=.\debug_print.c
# End Source File
# Begin Source File

SOURCE=.\ini_file.c
# End Source File
# Begin Source File

SOURCE=.\mdi_crap.c
# End Source File
# Begin Source File

SOURCE=.\messagebox.c
# End Source File
# Begin Source File

SOURCE=.\resize.c
# End Source File
# Begin Source File

SOURCE=.\servers.c
# End Source File
# Begin Source File

SOURCE=.\shell_menu.c
# End Source File
# Begin Source File

SOURCE=.\slimIRC.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\chan_modes.h
# End Source File
# Begin Source File

SOURCE=.\channel_window_stuff.h
# End Source File
# Begin Source File

SOURCE=.\dcc_window_stuff.h
# End Source File
# Begin Source File

SOURCE=.\file_logging.h
# End Source File
# Begin Source File

SOURCE=.\help.h
# End Source File
# Begin Source File

SOURCE=.\ircstuff.h
# End Source File
# Begin Source File

SOURCE=.\lua_specific_funcs.h
# End Source File
# Begin Source File

SOURCE=.\pragma.h
# End Source File
# Begin Source File

SOURCE=.\privmsg_window_stuff.h
# End Source File
# Begin Source File

SOURCE=.\ram_ini_file.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\server_window_stuff.h
# End Source File
# Begin Source File

SOURCE=.\libirc\src\session.h
# End Source File
# Begin Source File

SOURCE=.\static_window.h
# End Source File
# Begin Source File

SOURCE=.\switchbar_stuff.h
# End Source File
# Begin Source File

SOURCE=.\window.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\lock.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\slimirc.ico
# End Source File
# End Group
# End Target
# End Project
