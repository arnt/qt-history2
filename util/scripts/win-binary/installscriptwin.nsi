!define PRODUCT_NAME "Qt"
!define PRODUCT_PUBLISHER "Trolltech"
!define PRODUCT_WEB_SITE "http://www.trolltech.com"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!include "MUI.nsh"
!include "qtlicense.nsh"
!include "qtenv.nsh"
!include "registeruiext.nsh"
!include "writeQtConf.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME

; License check
!insertmacro TT_LICENSE_PAGE_SHOW

; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Environment setting page
!insertmacro TT_QTENV_PAGE_SHOW

; Register ui files
!insertmacro TT_UI_PAGE_SHOW

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION "ShowReadMe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

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
  CreateDirectory "$SMPROGRAMS\Qt"
  CreateShortCut "$SMPROGRAMS\Qt\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\Qt\Uninstall.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$SMPROGRAMS\Qt\${PRODUCT_NAME} ${PRODUCT_VERSION} Command Prompt.lnk" "%COMSPEC%" '/k "$INSTDIR\bin\qtvars.bat vsvars"'
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
  #registering ui extension
  !insertmacro TT_UI_REGISTER
  
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
SectionEnd

Function .onInit
  !insertmacro TT_LICENSE_PAGE_INIT
  !insertmacro TT_QTENV_PAGE_INIT
  !insertmacro TT_UI_PAGE_INIT
FunctionEnd

Function ShowReadMe
  Exec 'notepad.exe "$INSTDIR\README"'
FunctionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$SMPROGRAMS\Qt\Uninstall.lnk"
  Delete "$SMPROGRAMS\Qt\Website.lnk"
  Delete "$SMPROGRAMS\Qt\${PRODUCT_NAME} ${PRODUCT_VERSION} Command Prompt.lnk"

  RMDir "$SMPROGRAMS\Qt"
  RMDir /r "$INSTDIR"

  #unregister ui extensions
  !insertmacro TT_UI_UNREGISTER
  
  #removing the environment variables
  !insertmacro TT_QTENV_UNREGISTER
  
  #removing the qt version for the integration
  DeleteRegKey HKCU "SOFTWARE\trolltech.com\Versions\${PRODUCT_VERSION}\"
  
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd
