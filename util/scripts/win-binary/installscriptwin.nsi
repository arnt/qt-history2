!define PRODUCT_NAME "Qt"
!define PRODUCT_PUBLISHER "Trolltech"
!define PRODUCT_WEB_SITE "http://www.trolltech.com"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!include "MUI.nsh"
!include "qtlicense.nsh"
!include "qtenv.nsh"
!include "writeQtConf.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "install.ico"
!define MUI_UNICON "install.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "qt-header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "qt-header.bmp"
!define MUI_HEADERIMAGE_RIGHT

!define MUI_WELCOMEFINISHPAGE_BITMAP "qt-wizard.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "qt-wizard.bmp"

!define MUI_FINISHPAGE_NOAUTOCLOSE

; Welcome page

!insertmacro MUI_PAGE_WELCOME

; License check
!insertmacro TT_LICENSE_PAGE_SHOW

; Directory page
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE "CheckDirectory"
!insertmacro MUI_PAGE_DIRECTORY

; Environment setting page
!insertmacro TT_QTENV_PAGE_SHOW

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION "ShowReadMe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language files
!insertmacro MUI_LANGUAGE "English"
; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${DISTNAME}.exe"

InstallDir "C:\${PRODUCT_NAME}\${PRODUCT_VERSION}"
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /r "${PACKAGEDIR}\*.*"
  
  ;clean up license files
  strcmp $DISPLAY_US_LICENSE "1" 0 NonUS
    File /oname=".LICENSE" "${PACKAGEDIR}\.LICENSE-US"
    goto End

  NonUS:
    File /oname=".LICENSE" "${PACKAGEDIR}\.LICENSE"

  End:
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}\Uninstall.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}\${PRODUCT_NAME} ${PRODUCT_VERSION} Command Prompt.lnk" "%COMSPEC%" '/k "$INSTDIR\bin\qtvars.bat vsvars"'
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
  #setting the environment variables
  !insertmacro TT_QTENV_REGISTER

  #creating the qt.conf file
  push "$INSTDIR"
  call MakeQtConfFile

  #creating the qtvars.bat file
  push "$INSTDIR"
  call MakeQtVarsFile
  
  #setting the qt version for the integration
  WriteRegStr HKCU "SOFTWARE\trolltech.com\Versions\${PRODUCT_VERSION}\" "InstallDir" "$INSTDIR"

  call PatchPrlFiles
  call PatchPdbFiles
SectionEnd

Function .onInit
  !insertmacro TT_LICENSE_PAGE_INIT
  !insertmacro TT_QTENV_PAGE_INIT
FunctionEnd

Function PatchPrlFiles
  #patching the prl files
  push $0
  push $1

  FindFirst $0 $1 "$INSTDIR\lib\*.prl"
  loop:
    StrCmp $1 "" done
    DetailPrint "Patching $1..."

    push "$INSTDIR\lib\$1"
    push ${QTBUILDDIR}
    push $INSTDIR
    call PatchPath

    FindNext $0 $1
    Goto loop
  done:
  pop $1
  pop $0
FunctionEnd

Function PatchPdbFiles
  #patching the pdb files
  push $0
  push $1

  FindFirst $0 $1 "$INSTDIR\bin\*.pdb"
  loop:
    StrCmp $1 "" done
    DetailPrint "Patching $1..."
    
    ;Patch only source files (.cpp files) Maybe add .c files???
    qtnsisext::PatchBinary "$INSTDIR\bin\$1" ${QTBUILDDIR} $INSTDIR ".cpp"
    FindNext $0 $1
    Goto loop
  done:
  pop $1
  pop $0
FunctionEnd

Function CheckDirectory
  push $0
  push $1
  push $2
  push $3
  StrLen $0 $INSTDIR
  StrLen $1 ${QTBUILDDIR}

  IntCmp $1 $0 0 errorInDirectory
  
  ;check for spaces
  StrCpy $2 "-1"
  StrCpy $3 ""

  loop:
  IntOp $2 $2 + 1 ;increase counter
  StrCpy $3 $INSTDIR "1" $2 ;get char
  StrCmp $3 "" directoryOk ; check for end
  StrCmp $3 " " errorInDirectory ;check for space
  goto loop

errorInDirectory:
  MessageBox MB_OK|MB_ICONEXCLAMATION "The installation path can't contain spaces or be more than 100 characters long."
  pop $3
  pop $2
  pop $1
  pop $0
  Abort
  goto done
  
directoryOk:
  pop $3
  pop $2
  pop $1
  pop $0
  
done:
FunctionEnd

Function ShowReadMe
  Exec 'notepad.exe "$INSTDIR\README"'
FunctionEnd

Section Uninstall
  DetailPrint "Removing start menu shortcuts"
  Delete "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}\Uninstall.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}\Website.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}\${PRODUCT_NAME} ${PRODUCT_VERSION} Command Prompt.lnk"

  RMDir "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}"
  
  DetailPrint "Removing installation directory"
  RMDir /r "$INSTDIR"

  #removing the environment variables
  !insertmacro TT_QTENV_UNREGISTER
  
  #removing the qt version for the integration
  DetailPrint "Removing registry entries"
  DeleteRegKey HKCU "SOFTWARE\trolltech.com\Versions\${PRODUCT_VERSION}\"
  
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd
