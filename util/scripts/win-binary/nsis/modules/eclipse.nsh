!ifdef MODULE_ECLIPSE

;------------------------------------------------------------------------------------------------
!macro ECLIPSE_INITIALIZE

!define ECLIPSE_LOCATION_KEY "EclipseLocation"
var ECLIPSE_LOCATION
var ECLIPSE_MINGW_LOCATION

!ifndef MODULE_ECLIPSE_INSTALLER
  !define MODULE_ECLIPSE_INSTALLER "${PRODUCT_NAME} v${PRODUCT_VERSION}"
!endif
!ifndef MODULE_ECLIPSE_ROOT
  !define MODULE_ECLIPSE_ROOT "${INSTALL_ROOT}\eclipse"
!endif
!ifndef MODULE_ECLIPSE_VERSION
  !define MODULE_ECLIPSE_VERSION ${PRODUCT_VERSION}
!endif

!define MODULE_ECLIPSE_QT_ID "com.trolltech.qt_4.2.0"

!define MODULE_ECLIPSE_QTSTARTUP_ID "com.trolltech.qtstartup_${MODULE_ECLIPSE_VERSION}"

!define MODULE_ECLIPSE_QTPROJECT_ID "com.trolltech.qtproject_${MODULE_ECLIPSE_VERSION}"
!define MODULE_ECLIPSE_QTPROJECT_LABEL "Qt Project Integration"
!define MODULE_ECLIPSE_QTPROJECT_INSTALLEDKEY "EclipseQtProjectInstalled"

!define MODULE_ECLIPSE_QTDESIGNER_ID "com.trolltech.qtdesigner_${MODULE_ECLIPSE_VERSION}"
!define MODULE_ECLIPSE_QTDESIGNER_LABEL "Qt Designer Integration"
!define MODULE_ECLIPSE_QTDESIGNER_INSTALLEDKEY "EclipseQtDesignerInstalled"
;; qdesignerplugins
;; !define MODULE_ECLIPSE_QTDESIGNERPLUGINS_ID "com.trolltech.qtdesignerplugins"

!define MODULE_ECLIPSE_QTINTEGRATIONHELP_ID "com.trolltech.qtintegrationhelp_${MODULE_ECLIPSE_VERSION}"
!define MODULE_ECLIPSE_QTINTEGRATIONHELP_LABEL "Qt Integration Help"
!define MODULE_ECLIPSE_QTINTEGRATIONHELP_INSTALLEDKEY "EclipseQtIntegrationHelpInstalled"

!define MODULE_ECLIPSE_QTREFERENCE_ID "com.trolltech.help_${MODULE_ECLIPSE_VERSION}"
!define MODULE_ECLIPSE_QTREFERENCE_LABEL "Qt Reference documentation"
!define MODULE_ECLIPSE_QTREFERENCE_INSTALLEDKEY "EclipseQtReferenceInstalled"

LangString ModuleEclipsePageTitle ${LANG_ENGLISH} "Eclipse Installation Location"
LangString ModuleEclipsePageDescription ${LANG_ENGLISH} "Select where eclipse is installed, and where MinGW is located."

!define MODULE_ECLIPSE_PAGE "eclipse.ini"
Page custom ModuleEclipsePageEnter ModuleEclipsePageExit

!include "includes\regsvr.nsh"

!macroend ;ECLIPSE_INITIALIZE

;------------------------------------------------------------------------------------------------
!macro ECLIPSE_SECTIONS

Section -PreEclipseSection
  WriteRegStr SHCTX "$PRODUCT_UNIQUE_KEY" "${ECLIPSE_LOCATION_KEY}" $ECLIPSE_LOCATION
SectionEnd

SectionGroup "Eclipse Integration"

Section "${MODULE_ECLIPSE_QTPROJECT_LABEL}" ECLIPSE_SEC01
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "${MODULE_ECLIPSE_QTPROJECT_INSTALLEDKEY}" 1
  SetOutPath "$ECLIPSE_LOCATION\plugins\"
  SetOverwrite ifnewer
  File "${MODULE_ECLIPSE_ROOT}\plugins\${MODULE_ECLIPSE_QT_ID}.jar"
  File "${MODULE_ECLIPSE_ROOT}\plugins\${MODULE_ECLIPSE_QTSTARTUP_ID}.jar"
  File "${MODULE_ECLIPSE_ROOT}\plugins\${MODULE_ECLIPSE_QTPROJECT_ID}.jar"

  Call InstallQtModules
  
  SetOutPath "$ECLIPSE_INSTDIR"
  SetOverwrite ifnewer
  File "${MODULE_ECLIPSE_ROOT}\bin\qtproparser.dll"
  File "${MODULE_ECLIPSE_ROOT}\bin\qtqrceditor.dll"
  
  push "$ECLIPSE_INSTDIR\qtproparser.dll"
  call RegEclipseSvr
  
  push "$ECLIPSE_INSTDIR\qtqrceditor.dll"
  call RegEclipseSvr
  
  IfFileExists "$ECLIPSE_MINGW_LOCATION\gcc.exe" 0 done
  Call MakeEclipseStartFile
  CreateShortCut "$SMPROGRAMS\$STARTMENU_STRING\Start Eclipse with MinGW.lnk" "%COMSPEC%" '/c "$ECLIPSE_INSTDIR\start.bat"'

  done:
SectionEnd

Section "${MODULE_ECLIPSE_QTDESIGNER_LABEL}" ECLIPSE_SEC02
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "${MODULE_ECLIPSE_QTDESIGNER_INSTALLEDKEY}" 1
  SetOutPath "$ECLIPSE_LOCATION\plugins\"
  SetOverwrite ifnewer
  File "${MODULE_ECLIPSE_ROOT}\plugins\${MODULE_ECLIPSE_QTDESIGNER_ID}.jar"
  
;; qdesignerplugins
;;  SetOutPath "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTDESIGNERPLUGINS_ID}\"
;;  SetOverwrite ifnewer
;;  File "${MODULE_ECLIPSE_ROOT}\plugins\${MODULE_ECLIPSE_QTDESIGNERPLUGINS_ID}\qt3supportwidgets.dll"
  
  Call InstallQtModules
  
  SetOutPath "$ECLIPSE_INSTDIR"
  SetOverwrite ifnewer
  File "${MODULE_ECLIPSE_ROOT}\bin\QtDesigner4.dll"
  File "${MODULE_ECLIPSE_ROOT}\bin\QtScript4.dll"
  File "${MODULE_ECLIPSE_ROOT}\bin\QtDesignerComponents4.dll"
  File "${MODULE_ECLIPSE_ROOT}\bin\qtdesigner.dll"
  
  push "$ECLIPSE_INSTDIR\qtdesigner.dll"
  call RegEclipseSvr
SectionEnd

Section "${MODULE_ECLIPSE_QTINTEGRATIONHELP_LABEL}" ECLIPSE_SEC03
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "${MODULE_ECLIPSE_QTINTEGRATIONHELP_INSTALLEDKEY}" 1
  SetOutPath "$ECLIPSE_LOCATION\plugins\"
  SetOverwrite ifnewer
  File "${MODULE_ECLIPSE_ROOT}\plugins\${MODULE_ECLIPSE_QTINTEGRATIONHELP_ID}.jar"
SectionEnd

Section "${MODULE_ECLIPSE_QTREFERENCE_LABEL}" ECLIPSE_SEC04
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "${MODULE_ECLIPSE_QTREFERENCE_INSTALLEDKEY}" 1
  SetOutPath "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTREFERENCE_ID}"
  SetOverwrite ifnewer
  File "${MODULE_ECLIPSE_ROOT}\plugins\com.trolltech.help\doc.zip"
  File "${MODULE_ECLIPSE_ROOT}\plugins\com.trolltech.help\plugin.xml"
  File "${MODULE_ECLIPSE_ROOT}\plugins\com.trolltech.help\qt.xml"
  SetOutPath "$ECLIPSE_LOCATION\features\${MODULE_ECLIPSE_QTREFERENCE_ID}"
  SetOverwrite ifnewer
  File "${MODULE_ECLIPSE_ROOT}\features\com.trolltech.help\feature.xml"
SectionEnd

SectionGroupEnd

; usage:
; push dll to register
; call RegEclipseSvr
Function RegEclipseSvr
  exch $0 ;filename

  ClearErrors
  push $0
  call RegSvr
  IfErrors 0 +2
    MessageBox MB_OK|MB_ICONEXCLAMATION 'Could not register "$0"'

  pop $0
FunctionEnd

#
# creates a start.bat file with mingw in the path
#
Function MakeEclipseStartFile
  push $0 ; file handle

  ClearErrors
  FileOpen $0 "$ECLIPSE_INSTDIR\start.bat" w
  IfErrors done
  
  FileWrite $0 "@echo off$\r$\n"
  FileWrite $0 "rem$\r$\n"
  FileWrite $0 "rem This file is generated by the installer$\r$\n"
  FileWrite $0 "rem$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "echo Setting up environment...$\r$\n"
  FileWrite $0 "echo -- Using MinGW in: $ECLIPSE_MINGW_LOCATION $\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "set PATH=$ECLIPSE_MINGW_LOCATION$\r$\n"
  FileWrite $0 "set PATH=%PATH%;%SystemRoot%\System32$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "echo Starting eclipse...$\r$\n"
  FileWrite $0 'call "$ECLIPSE_LOCATION\eclipse.exe"'
  FileWrite $0 "$\r$\n"
  FileClose $0
  
  done:
  pop $0
FunctionEnd

Function InstallQtModules
  SetOutPath "$ECLIPSE_INSTDIR"
  SetOverwrite ifnewer
  File "${MODULE_ECLIPSE_ROOT}\bin\msvcp80.dll"
  File "${MODULE_ECLIPSE_ROOT}\bin\msvcr80.dll"
  File "${MODULE_ECLIPSE_ROOT}\bin\QtCore4.dll"
  File "${MODULE_ECLIPSE_ROOT}\bin\QtGui4.dll"
  File "${MODULE_ECLIPSE_ROOT}\bin\QtXml4.dll"
;; qdesignerplugins
;;  File "${MODULE_ECLIPSE_ROOT}\bin\QtSql4.dll"
;;  File "${MODULE_ECLIPSE_ROOT}\bin\QtNetwork4.dll"
;;  File "${MODULE_ECLIPSE_ROOT}\bin\Qt3Support4.dll"
FunctionEnd

Function ModuleEclipsePageEnter
  !insertmacro MUI_HEADER_TEXT "$(ModuleEclipsePageTitle)" "$(ModuleEclipsePageDescription)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "${MODULE_ECLIPSE_PAGE}"
FunctionEnd

Function ModuleEclipsePageExit
  push $0
  push $1

  !insertmacro MUI_INSTALLOPTIONS_READ $0 "${MODULE_ECLIPSE_PAGE}" "Field 5" "State"
  IfFileExists "$0\eclipse.exe" eclipse_found
  MessageBox MB_OK|MB_ICONSTOP "$0\eclipse.exe not found!$\nPlease select a valid installation directory."
  Goto failed
  eclipse_found:
  
  ClearErrors
  FileOpen $1 "$0\plugins\com.trolltech.writetest" a
  IfErrors 0 has_write_access
  MessageBox MB_OK|MB_ICONSTOP "Can't write to $0\plugins.$\nPlease select a valid installation directory."
  Goto failed
  has_write_access:

  FileClose $1
  Delete "$0\plugins\com.trolltech.writetest"
  StrCpy $ECLIPSE_LOCATION $0
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "${MODULE_ECLIPSE_PAGE}" "Field 3" "State"
  StrCpy $ECLIPSE_MINGW_LOCATION $0
  
  Goto done
  failed:
  pop $1
  pop $0
  Abort
  
  done:
  pop $1
  pop $0
FunctionEnd

!macroend ;ECLIPSE_SECTIONS

;------------------------------------------------------------------------------------------------
!macro ECLIPSE_DESCRIPTION
!ifdef ECLIPSE_SEC01
  !insertmacro MUI_DESCRIPTION_TEXT ${ECLIPSE_SEC01} "This installs the ${MODULE_ECLIPSE_QTPROJECT_LABEL} into Eclipse."
!endif
!ifdef ECLIPSE_SEC02
  !insertmacro MUI_DESCRIPTION_TEXT ${ECLIPSE_SEC02} "This installs the ${MODULE_ECLIPSE_QTDESIGNER_LABEL} into Eclipse."
!endif
!ifdef ECLIPSE_SEC03
  !insertmacro MUI_DESCRIPTION_TEXT ${ECLIPSE_SEC03} "This installs the ${MODULE_ECLIPSE_QTINTEGRATIONHELP_LABEL} into Eclipse."
!endif
!ifdef ECLIPSE_SEC04
  !insertmacro MUI_DESCRIPTION_TEXT ${ECLIPSE_SEC04} "This installs the ${MODULE_ECLIPSE_QTREFERENCE_LABEL} into Eclipse."
!endif
!macroend

;------------------------------------------------------------------------------------------------
!macro ECLIPSE_STARTUP
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${MODULE_ECLIPSE_PAGE}"
  SectionSetFlags ${ECLIPSE_SEC01} 17
  SectionSetFlags ${ECLIPSE_SEC02} 1
  SectionSetFlags ${ECLIPSE_SEC03} 1
  
  strcpy $ECLIPSE_INSTDIR "$PROGRAMFILES\Trolltech\Eclipse"
!macroend ;ECLIPSE_STATUP

;------------------------------------------------------------------------------------------------
!macro ECLIPSE_FINISH
!macroend

;------------------------------------------------------------------------------------------------
!macro ECLIPSE_UNSTARTUP
  ReadRegStr $ECLIPSE_LOCATION SHCTX "$PRODUCT_UNIQUE_KEY" "${ECLIPSE_LOCATION_KEY}"
  !insertmacro ConfirmOnRemove "${MODULE_ECLIPSE_QTPROJECT_INSTALLEDKEY}" "${MODULE_ECLIPSE_QTPROJECT_LABEL}"
  !insertmacro ConfirmOnRemove "${MODULE_ECLIPSE_QTDESIGNER_INSTALLEDKEY}" "${MODULE_ECLIPSE_QTDESIGNER_LABEL}"
  !insertmacro ConfirmOnRemove "${MODULE_ECLIPSE_QTINTEGRATIONHELP_INSTALLEDKEY}" "${MODULE_ECLIPSE_QTINTEGRATIONHELP_LABEL}"
  !insertmacro ConfirmOnRemove "${MODULE_ECLIPSE_QTREFERENCE_INSTALLEDKEY}" "${MODULE_ECLIPSE_QTREFERENCE_LABEL}"
!macroend

;------------------------------------------------------------------------------------------------
!macro ECLIPSE_UNINSTALL
Section un."Eclipse Integration"
  push $0

  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "${MODULE_ECLIPSE_QTPROJECT_INSTALLEDKEY}"
  intcmp $0 1 0 DoneUnInstallQtProject
    push "$ECLIPSE_INSTDIR\qtproparser.dll"
    call un.RegSvr
    push "$ECLIPSE_INSTDIR\qtqrceditor.dll"
    call un.RegSvr
  
    Delete "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QT_ID}.jar"
    Delete "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTSTARTUP_ID}.jar"
    Delete "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTPROJECT_ID}.jar"
    Delete "$ECLIPSE_INSTDIR\qtproparser.dll"
    Delete "$ECLIPSE_INSTDIR\qtqrceditor.dll"
    Delete "$SMPROGRAMS\$STARTMENU_STRING\Start Eclipse with MinGW.lnk"
    Delete "$ECLIPSE_INSTDIR\start.bat"
  DoneUnInstallQtProject:

  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" ${MODULE_ECLIPSE_QTDESIGNER_INSTALLEDKEY}
  intcmp $0 1 0 DoneUnInstallQtDesigner
    push "$ECLIPSE_INSTDIR\qtdesigner.dll"
    call un.RegSvr

;; qdesignerplugins  
;;    Delete "${MODULE_ECLIPSE_ROOT}\plugins\${MODULE_ECLIPSE_QTDESIGNERPLUGINS_ID}\qt3supportwidgets.dll"

    Delete "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTDESIGNER_ID}.jar"
    Delete "$ECLIPSE_INSTDIR\QtDesigner4.dll"
    Delete "$ECLIPSE_INSTDIR\QtScript4.dll"
    Delete "$ECLIPSE_INSTDIR\QtDesignerComponents4.dll"
    Delete "$ECLIPSE_INSTDIR\qtdesigner.dll"
  DoneUnInstallQtDesigner:
  
  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "${MODULE_ECLIPSE_QTINTEGRATIONHELP_INSTALLEDKEY}"
  intcmp $0 1 0 DoneUnInstallQtIntegrationHelp
    Delete "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTINTEGRATIONHELP_ID}.jar"
  DoneUnInstallQtIntegrationHelp:

  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "${MODULE_ECLIPSE_QTREFERENCE_INSTALLEDKEY}"
  intcmp $0 1 0 DoneUnInstallQtReference
    Delete "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTREFERENCE_ID}\doc.zip"
    Delete "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTREFERENCE_ID}\plugin.xml"
    Delete "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTREFERENCE_ID}\qt.xml"
    RMDir "$ECLIPSE_LOCATION\plugins\${MODULE_ECLIPSE_QTREFERENCE_ID}"
    Delete "$ECLIPSE_LOCATION\features\${MODULE_ECLIPSE_QTREFERENCE_ID}\feature.xml"
    RMDir "$ECLIPSE_LOCATION\features\${MODULE_ECLIPSE_QTREFERENCE_ID}"
  DoneUnInstallQtReference:
  
  Delete "$ECLIPSE_INSTDIR\msvcp80.dll"
  Delete "$ECLIPSE_INSTDIR\msvcr80.dll"
  Delete "$ECLIPSE_INSTDIR\QtCore4.dll"
  Delete "$ECLIPSE_INSTDIR\QtGui4.dll"
  Delete "$ECLIPSE_INSTDIR\QtXml4.dll"
;; qdesignerplugins
;;  Delete "$ECLIPSE_INSTDIR\QtSql4.dll"
;;  Delete "$ECLIPSE_INSTDIR\QtNetwork4.dll"
;;  Delete "$ECLIPSE_INSTDIR\Qt3Support4.dll"
  
  RMDir "$ECLIPSE_INSTDIR"

  pop $0
SectionEnd
!macroend ;ECLIPSE_UNINSTALL

;------------------------------------------------------------------------------------------------
!macro ECLIPSE_UNFINISH
!macroend

!else ;MODULE_ECLIPSE
!macro ECLIPSE_INITIALIZE
!macroend
!macro ECLIPSE_SECTIONS
!macroend
!macro ECLIPSE_DESCRIPTION
!macroend
!macro ECLIPSE_STARTUP
!macroend
!macro ECLIPSE_FINISH
!macroend
!macro ECLIPSE_UNSTARTUP
!macroend
!macro ECLIPSE_UNINSTALL
!macroend
!macro ECLIPSE_UNFINISH
!macroend
!endif ;MODULE_ECLIPSE

