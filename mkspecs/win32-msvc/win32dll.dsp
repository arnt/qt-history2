# Microsoft Developer Studio Project File - Name="$$MSVCDSP_PROJECT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version $$MSVCDSP_VER
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=$$MSVCDSP_PROJECT - Win32 $$MSVCDSP_CONFIGMODE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "$$MSVCDSP_PROJECT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "$$MSVCDSP_PROJECT.mak" CFG="$$MSVCDSP_PROJECT - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "$$MSVCDSP_PROJECT - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "$$MSVCDSP_PROJECT - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "$$MSVCDSP_PROJECT - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD CPP $$MSVCDSP_MTDEF /W3 /O1 $$MSVCDSP_INCPATH /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" $$MSVCDSP_DEFINES $$MSVCDSP_RELDEFS /FD /c $$MSVCDSP_CXXFLAGS
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib $$MSVCDSP_LIBS /dll $$MSVCDSP_VERSION $$MSVCDSP_TARGET $$MSVCDSP_LFLAGS
$$MSVCDSP_COPY_DLL

!ELSEIF  "$(CFG)" == "$$MSVCDSP_PROJECT - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD CPP $$MSVCDSP_MTDEFD /W3 /Gm $$MSVCDSP_DEBUG_OPT /Od $$MSVCDSP_INCPATH /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" $$MSVCDSP_DEFINES /FD /c $$MSVCDSP_CXXFLAGS
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib $$MSVCDSP_LIBS /dll /debug $$MSVCDSP_VERSION $$MSVCDSP_TARGET $$MSVCDSP_LFLAGS /pdbtype:sept
$$MSVCDSP_COPY_DLL

!ENDIF 

# Begin Target

# Name "$$MSVCDSP_PROJECT - Win32 Release"
# Name "$$MSVCDSP_PROJECT - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
$$MSVCDSP_SOURCES
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
$$MSVCDSP_HEADERS
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Forms"
$$MSVCDSP_FORMS
# Prop Default_Filter "ui"
# End Group
# Begin Group "Lexables"
$$MSVCDSP_LEXSOURCES
# Prop Default_Filter "l"
# End Group
# Begin Group "Yaccables"
$$MSVCDSP_YACCSOURCES
# Prop Default_Filter "y"
# End Group
# Begin Group "Generated"
$$MSVCDSP_MOCSOURCES
$$MSVCDSP_FORMSOURCES
$$MSVCDSP_FORMHEADERS
$$MSVCDSP_IMAGES
# Prop Default_Filter "moc"
# End Group
# End Target
# End Project
