!define GW_DOWNLOAD_INI_FILE "gwdownload.ini"
!define GW_MIRROR_INI_FILE "gwmirror.ini"
!define GW_MIRROR_LIST_URL "ftp://ftp.trolltech.com/misc"
!define GW_MINGW_VERSION "3.4.2"
!define GW_DOWNLOAD_FILE "MinGW-${GW_MINGW_VERSION}"
!define GW_RUNTIME_LIB "mingw*.dll"
!define GW_LICENSE_FILE "C:\MinGW\COPYING"

var GW_DO_DOWNLOAD
var GW_DO_DOWNLOAD_SOURCE
var GW_MIRRORS
var GW_INSTALLATION_OK
var GW_INSTALL_DIR

LangString GWDownloadTitle ${LANG_ENGLISH} "MinGW Checking"
LangString GWDownloadTitleDescription ${LANG_ENGLISH} "You need MinGW to be able to compile Qt applications."
LangString GWMirrorTitle ${LANG_ENGLISH} "MinGW Download Mirror"
LangString GWMirrorTitleDescription ${LANG_ENGLISH} "Select a download mirror."

!macro GW_PAGE_INIT
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${GW_DOWNLOAD_INI_FILE}"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${GW_MIRROR_INI_FILE}"

  !insertmacro MUI_INSTALLOPTIONS_WRITE "${GW_DOWNLOAD_INI_FILE}" "Field 3" "State" "C:\MinGW"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "${GW_DOWNLOAD_INI_FILE}" "Field 6" "State" "C:\MinGW"

  strcpy $GW_DO_DOWNLOAD "no"
  strcpy $GW_DO_DOWNLOAD_SOURCE "no"
!macroend

!macro GW_PAGE_INSERT
  Page custom ShowGWDownloadPage ExitGWDownloadPage
  !define MUI_PAGE_CUSTOMFUNCTION_PRE ShowMinGWLicense
  !define MUI_LICENSEPAGE_TEXT_TOP "MinGW License Information"
  !insertmacro MUI_PAGE_LICENSE "${GW_LICENSE_FILE}"
  Page custom ShowGWMirrorPage ExitGWMirrorPage
!macroend

Function ShowGWDownloadPage
  strcmp $GW_INSTALLATION_OK "yes" 0 +2
  Abort

  !insertmacro MUI_HEADER_TEXT "$(GWDownloadTitle)" "$(GWDownloadTitleDescription)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "${GW_DOWNLOAD_INI_FILE}"
FunctionEnd

Function ShowGWMirrorPage
  strcmp $GW_DO_DOWNLOAD "yes" +2
  Abort

  !insertmacro MUI_HEADER_TEXT "$(GWMirrorTitle)" "$(GWMirrorTitleDescription)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "${GW_MIRROR_INI_FILE}"
FunctionEnd

Function ShowMinGWLicense
  strcmp $GW_DO_DOWNLOAD "yes" +2
  Abort
FunctionEnd

Function ExitGWDownloadPage
  push $0
  push $1
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "${GW_DOWNLOAD_INI_FILE}" "Field 8" "State"
  strcmp "$0" "0" noDownload doDownload
  
doDownload:
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "${GW_DOWNLOAD_INI_FILE}" "Field 6" "State"
  strcmp $0 "" 0 +3
    MessageBox MB_ICONEXCLAMATION|MB_OK "You need to specify an installation directory!"
    goto tryAgain

  strcpy $GW_INSTALL_DIR $0
  strcpy $GW_DO_DOWNLOAD "yes"
  CreateDirectory "$INSTDIR\downloads"
  InetLoad::load /BANNER "Mirror Download" "Downloading mirrors from server..." "${GW_MIRROR_LIST_URL}/${GW_DOWNLOAD_FILE}.mirrors" "$INSTDIR\downloads\${GW_DOWNLOAD_FILE}.mirrors" /END
  Pop $1 ;Get the return value

  StrCmp $1 "OK" +3
    MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "Was not able to download mirror list ($1)." IDRETRY tryAgain 0
    Quit

  call ReadMirrors
  !insertmacro MUI_INSTALLOPTIONS_WRITE ${GW_MIRROR_INI_FILE} "Field 3" "ListItems" "$GW_MIRRORS"
  goto done
    
noDownload:
  strcpy $GW_DO_DOWNLOAD "no"
  strcpy $GW_DO_DOWNLOAD_SOURCE "no"
  call DoMinGWChecking
  strcmp $GW_INSTALLATION_OK "yes" done
  MessageBox MB_ICONEXCLAMATION|MB_YESNO "It is a problem with your MinGW installation:$\r$\n$GW_INSTALLATION_OK$\r$\nDo you still want to continue? (Your installation may not work)" IDNO tryAgain
  goto done
  
tryAgain:
  pop $1
  pop $0
  Abort
    
done:
  pop $1
  pop $0
FunctionEnd

Function ExitGWMirrorPage
  push $0
  push $2
  push $1

  !insertmacro MUI_INSTALLOPTIONS_READ $0 "${GW_MIRROR_INI_FILE}" "Field 3" "State"
  strcmp "$0" "" 0 +3
    MessageBox MB_ICONEXCLAMATION|MB_OK "You must select a mirror to download from!"
    goto tryAgain

  push $0
  call GetMirror
  pop $0
  
  InetLoad::load /BANNER "MinGW Download" "Downloading MinGW from server..." "$0/${GW_DOWNLOAD_FILE}.exe" "$INSTDIR\downloads\${GW_DOWNLOAD_FILE}.exe" /END
  Pop $2 ;get the return value

  StrCmp $2 "OK" +3
    MessageBox MB_ICONEXCLAMATION|MB_OK "Was not able to download MinGW ($2). Please try another mirror."
    Goto tryAgain
    
  !insertmacro MUI_INSTALLOPTIONS_READ $1 "${GW_MIRROR_INI_FILE}" "Field 2" "State"
  strcmp "$1" "0" done
 
  InetLoad::load /BANNER "MinGW Sources Download" "Downloading MinGW Sources from server..." "$0/${GW_DOWNLOAD_FILE}-src.exe" "$INSTDIR\downloads\${GW_DOWNLOAD_FILE}-src.exe" /END
  Pop $2
  
  strcpy $GW_DO_DOWNLOAD_SOURCE "yes"

  StrCmp $2 "OK" +3
    MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "Was not able to download MinGW sources ($2). Please try another mirror?" IDRETRY tryAgain 0
    Quit
    
  goto done
    
tryAgain:
;  SetOutPath "$INSTDIR"
  pop $1
  pop $2
  pop $0
  Abort

done:
;  SetOutPath "$INSTDIR"
  pop $1
  pop $2
  pop $0
FunctionEnd

Function ReadMirrors
  push $0 ;file handle
  push $1 ;line

  ClearErrors
  FileOpen $0 "$INSTDIR\downloads\${GW_DOWNLOAD_FILE}.mirrors" r
  IfErrors done

  strcpy $GW_MIRRORS ""

nextline:
  FileRead $0 $1
  IfErrors done
  push $1
  call RemoveNewLine
  pop $1
  strcpy $GW_MIRRORS "$GW_MIRRORS|$1"
  FileRead $0 $1 ;Jump over next line
  IfErrors done
  goto nextline

done:
  FileClose $0
  strlen $1 $GW_MIRRORS
  intcmp $1 0 failed failed cleanup
  
failed:
  MessageBox MB_ICONSTOP|MB_OK "Unable to parse mirror list, exiting!"
  Quit

cleanup:
  pop $1
  pop $0
FunctionEnd

#this just removes the last two chars
Function RemoveNewLine
exch $0
push $1
push $2

strlen $1 $0
intop $1 $1 - 1
strcpy $2 $0 1 $1 ;get last char

strcmp "$2" "$\n" 0 +2
intop $1 $1 - 1

strcpy $2 $0 1 $1 ;get last char
strcmp "$2" "$\r" 0 +2
intop $1 $1 - 1

intop $1 $1 + 1
strcpy $0 $0 $1

pop $2
pop $1
exch $0
FunctionEnd

#push serverid
#call GetMirror
#pop server
Function GetMirror
  exch $1 ;id
  push $0 ;file handle
  push $2 ;line
  push $3 ;tmp
  
  strcpy $3 ""

  ClearErrors
  FileOpen $0 "$INSTDIR\downloads\${GW_DOWNLOAD_FILE}.mirrors" r
  IfErrors done

nextline:
  FileRead $0 $2
  IfErrors done
  push $2
  call RemoveNewLine
  pop $2
  strcmp $1 $2 0 nextline
  FileRead $0 $3
  IfErrors done
  push $3
  call RemoveNewLine
  pop $3

done:
  strcpy $1 $3
  FileClose $0
  strlen $2 $1
  intcmp $2 0 failed failed cleanup

failed:
  MessageBox MB_ICONSTOP|MB_OK "Unable to parse mirror list, exiting!"
  Quit

cleanup:
  pop $3
  pop $2
  pop $0
  exch $1
FunctionEnd

Function DoMinGWChecking
  push $0
  
  ### update with plugin
  strcpy $GW_INSTALLATION_OK "yes"
  strcpy $GW_INSTALL_DIR "C:\MinGW" ;fallback dir
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "${GW_DOWNLOAD_INI_FILE}" "Field 3" "State"
  strcmp "$0" "" +2
    strcpy $GW_INSTALL_DIR $0
    
  IfFileExists "$GW_INSTALL_DIR\bin\g++.exe" +3 0
    strcpy $GW_INSTALLATION_OK "MinGW not found!"
    goto DoneChecking
    
  ; check w32api.h
  push $GW_INSTALL_DIR
  gwnsisext::HasValidWin32Library
  pop $0
  strcmp "$0" "1" +3 0
    strcpy $GW_INSTALLATION_OK "Could not find a valid win32 library."
    goto DoneChecking
    
  ; check w32api.h
  push $GW_INSTALL_DIR
  gwnsisext::GetMinGWVersion
  pop $0
  strcmp "$0" "${GW_MINGW_VERSION}" +3 0
    strcpy $GW_INSTALLATION_OK "MinGW version found does not match ${GW_MINGW_VERSION} (Found version $0)."
    goto DoneChecking

DoneChecking:
  pop $0
FunctionEnd

#
# creates a qtvars.bat file in $QTDIR\bin
# push "c:\qt"  #QTDIR
# call MakeQtVarsFile
#
Function MakeMinGWEnvFile
  push $0 ; file handle
  push $1 ; SHELL
  strcpy $1 "rem "

  IfFileExists "$GW_INSTALL_DIR\bin\sh.exe" 0 +3
    strcpy $1 ""
    goto WriteMinGWEnv
    
  gwnsisext::ShInPath
  pop $0
  strcmp $0 "1" 0 +3
    strcpy $1 ""
    goto WriteMinGWEnv

WriteMinGWEnv:
  ClearErrors
  FileOpen $0 "$INSTDIR\bin\qtvars.bat" w
  IfErrors done
  FileWrite $0 "@echo off$\r$\n"
  FileWrite $0 "rem$\r$\n"
  FileWrite $0 "rem This file is generated$\r$\n"
  FileWrite $0 "rem$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "echo Setting up a Qt environment...$\r$\n"
  FileWrite $0 "echo -- QTDIR set to $INSTDIR$\r$\n"
  FileWrite $0 "echo -- Added $GW_INSTALL_DIR\bin to PATH$\r$\n"
  FileWrite $0 "echo -- Added $INSTDIR\bin to PATH$\r$\n"
  FileWrite $0 "echo -- QMAKESPEC set to win32-g++$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "set QTDIR=$INSTDIR$\r$\n"
  FileWrite $0 "set PATH=$GW_INSTALL_DIR\bin;%PATH%$\r$\n"
  FileWrite $0 "set PATH=$INSTDIR\bin;%PATH%$\r$\n"
  FileWrite $0 "set QMAKESPEC=win32-g++$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "rem If sh.exe is in the path, then comment out this:$\r$\n"
  FileWrite $0 "$1set MINGW_IN_SHELL=1$\r$\n"
  FileWrite $0 "$1echo -- Using MinGW in shell$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "cd %QTDIR%$\r$\n"

  FileWrite $0 "$\r$\n"
  FileWrite $0 'if not "%1"=="compile_debug" goto END$\r$\n'
  FileWrite $0 "echo This will configure and compile qt in debug. The release libraries will not be recompiled.$\r$\n"
  FileWrite $0 "pause$\r$\n"
  FileWrite $0 "configure -plugin-sql-sqlite -plugin-sql-odbc -qt-style-windowsxp -qt-libpng -qt-libjpeg$\r$\n"
  FileWrite $0 "cd %QTDIR%\src$\r$\n"
  FileWrite $0 "mingw32-make debug$\r$\n"
  FileWrite $0 ":END$\r$\n"
  FileClose $0

done:
  pop $1
  pop $0
FunctionEnd
