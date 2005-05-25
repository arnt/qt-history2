!define PRODUCT_NAME "Qt"
!define PRODUCT_PUBLISHER "Trolltech"
!define PRODUCT_WEB_SITE "http://www.trolltech.com"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} ${PRODUCT_VERSION}"
!define LICENSEE_REPLACENAME "Beta Tester"

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
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH

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
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show Documentation"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION "ShowDocumentation"
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

InstallDir "C:\${PRODUCT_NAME}\${PRODUCT_VERSION}\"
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /r "${PACKAGEDIR}\*.*"
  
  ;clean up license files
  SetDetailsPrint none
  strcmp $DISPLAY_US_LICENSE "1" 0 NonUS
    Delete "$INSTDIR\.LICENSE"
    Rename "$INSTDIR\.LICENSE-US" "$INSTDIR\.LICENSE"
    Goto End

  NonUS:
    Delete "$INSTDIR\.LICENSE-US"

  End:
  SetDetailsPrint both
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}"

  #call AddDemoShortCuts
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Assistant.lnk" "$INSTDIR\bin\assistant.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Designer.lnk" "$INSTDIR\bin\designer.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Linguist.lnk" "$INSTDIR\bin\linguist.exe"
  
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\${PRODUCT_NAME} ${PRODUCT_VERSION} Command Prompt.lnk" "%COMSPEC%" '/k "$INSTDIR\bin\qtvars.bat vsvars"'
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Examples and Demos.lnk" "$INSTDIR\bin\qtdemo.exe"
  
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Trolltech.com.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Uninstall Qt ${PRODUCT_VERSION}.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  
  StrCmp $RUNNING_AS_ADMIN "false" NoAdmin
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "SOFTWARE\trolltech\Versions\${PRODUCT_VERSION}\" "InstallDir" "$INSTDIR"
  Goto DoneWriteReg
  NoAdmin:
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKCU "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKCU "SOFTWARE\trolltech\Versions\${PRODUCT_VERSION}\" "InstallDir" "$INSTDIR"
  DoneWriteReg:
  
  #setting the environment variables
  !insertmacro TT_QTENV_REGISTER

  #creating the qt.conf file
  #push "$INSTDIR"
  #call MakeQtConfFile

  #creating the qtvars.bat file
  push "$INSTDIR"
  call MakeQtVarsFile

  call PatchPrlFiles
  call PatchBinaryFiles
  
  #patch the licencee information
  DetailPrint "Patching license information..."

  #### LICENSEE_REPLACENAME must reflect license used when compiling
  push "$INSTDIR\include\QtCore\qconfig.h"
  push "${LICENSEE_REPLACENAME}"
  push "$LICENSEE"
  call PatchPath
  
  push $0
  push $1
  FindFirst $0 $1 "$INSTDIR\bin\QtCore*.dll"
  StrCmp $1 "" ErrorPatching
  qtnsisext::PatchBinary /NOUNLOAD "$INSTDIR\bin\$1" "qt_lcnsuser=" "qt_lcnsuser=$LICENSEE"
  
  FindNext $0 $1
  StrCmp $1 "" ErrorPatching
  qtnsisext::PatchBinary "$INSTDIR\bin\$1" "qt_lcnsuser=" "qt_lcnsuser=$LICENSEE"

  ErrorPatching:
  pop $1
  pop $0
  ####

  DetailPrint "Please wait while creating project files for examples..."
  nsExec::Exec "$INSTDIR\bin\qtvars.bat setup"
SectionEnd

Function .onInit
  call SetAdminVar
  !insertmacro TT_LICENSE_PAGE_INIT
  !insertmacro TT_QTENV_PAGE_INIT
FunctionEnd

# Function AddDemoShortCuts
#   push $0
#   push $1
#
#   SetOutPath "$INSTDIR\bin" ;this becomes the Start In path (where the qt libs are)
#   CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}\Demos"
#
#   FindFirst $0 $1 "$INSTDIR\demos\*.*"
#   loop:
#     StrCmp $1 "" done
#     IfFileExists "$INSTDIR\demos\$1\$1.exe" 0 +2
#     CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}\Demos\$1.lnk" "$INSTDIR\demos\$1\$1.exe"
#     IfFileExists "$INSTDIR\demos\$1\release\$1.exe" 0 +2
#     CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME} ${PRODUCT_VERSION}\Demos\$1.lnk" "$INSTDIR\demos\$1\release\$1.exe"
#     FindNext $0 $1
#     Goto loop
#
#   done:
#
#   SetOutPath "$INSTDIR"
#   pop $1
#   pop $0
# FunctionEnd

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

Function PatchBinaryFiles
  #patching the pdb files
  push $0
  push $1

  FindFirst $0 $1 "$INSTDIR\bin\*.pdb"
  loop:
    StrCmp $1 "" done
    DetailPrint "Patching $1..."
    
    StrCmp ${FORCE_MAKESPEC} "vc60" +3
    qtnsisext::PatchVC7Binary "$INSTDIR\bin\$1" "${QTBUILDDIR}" "$INSTDIR"
    Goto +2
    qtnsisext::PatchVC6Binary "$INSTDIR\bin\$1" "${QTBUILDDIR}" "$INSTDIR"
    FindNext $0 $1
    Goto loop
  done:

  DetailPrint "Patching qtmaind.lib..."
  StrCmp ${FORCE_MAKESPEC} "vc60" +3
  qtnsisext::PatchVC7Binary "$INSTDIR\lib\qtmaind.lib" "${QTBUILDDIR}" "$INSTDIR"
  Goto +2
  qtnsisext::PatchVC6Binary "$INSTDIR\lib\qtmaind.lib" "${QTBUILDDIR}" "$INSTDIR"

  DetailPrint "Patching paths in qmake..."
  push "$INSTDIR\bin\qmake.exe"
  call PatchBinaryPaths
  
  DetailPrint "Patching paths in core..."
  FindFirst $0 $1 "$INSTDIR\bin\QtCore*.dll"
  StrCmp $1 "" ErrorPatching
  push "$INSTDIR\bin\$1"
  call PatchBinaryPaths

  FindNext $0 $1
  StrCmp $1 "" ErrorPatching
  push "$INSTDIR\bin\$1"
  call PatchBinaryPaths

  ErrorPatching:
  
  pop $1
  pop $0
FunctionEnd

### check if right
Function PatchBinaryPaths
  exch $0
  push $1
  
  qtnsisext::PatchBinary /NOUNLOAD $0 "qt_prfxpath=" "qt_prfxpath=$INSTDIR"
  qtnsisext::PatchBinary /NOUNLOAD $0 "qt_docspath=" "qt_docspath=$INSTDIR\doc"
  qtnsisext::PatchBinary /NOUNLOAD $0 "qt_hdrspath=" "qt_hdrspath=$INSTDIR\include"
  qtnsisext::PatchBinary /NOUNLOAD $0 "qt_libspath=" "qt_libspath=$INSTDIR\lib"
  qtnsisext::PatchBinary /NOUNLOAD $0 "qt_binspath=" "qt_binspath=$INSTDIR\bin"
  qtnsisext::PatchBinary /NOUNLOAD $0 "qt_plugpath=" "qt_plugpath=$INSTDIR\plugins"
  qtnsisext::PatchBinary /NOUNLOAD $0 "qt_datapath=" "qt_datapath=$INSTDIR"
  qtnsisext::PatchBinary /NOUNLOAD $0 "qt_trnspath=" "qt_trnspath=$INSTDIR\translations"
  qtnsisext::PatchBinary /NOUNLOAD $0 "qt_xmplpath=" "qt_xmplpath=$INSTDIR\examples"
  qtnsisext::PatchBinary $0 "qt_demopath=" "qt_demopath=$INSTDIR\demos"
  
  pop $1
  pop $0
FunctionEnd

Function CheckDirectory
  push $0
  push $1
  push $2
  push $3
  
  ; check if qt is already installed
  IfFileExists "$INSTDIR\bin\qmake.exe" 0 +2
  IfFileExists "$INSTDIR\uninst.exe" qtExistsError
  
  GetInstDirError $0
  IntCmp 0 $0 0 instDirError
  
  StrLen $0 $INSTDIR
  StrLen $1 ${QTBUILDDIR}

  IntCmp $1 $0 0 directoryToLong
  
  ;check for spaces
  StrCpy $2 "-1"
  StrCpy $3 ""

  loop:
  IntOp $2 $2 + 1 ;increase counter
  StrCpy $3 $INSTDIR "1" $2 ;get char
  StrCmp $3 "" directoryOk ; check for end
  StrCmp $3 " " spaceInDirectory ;check for space
  goto loop
  
qtExistsError:
  MessageBox MB_OK|MB_ICONEXCLAMATION "Qt is already installed in this directory. Please uninstall the previous version and try again."
  Goto errorInDirectory

instDirError:
  MessageBox MB_OK|MB_ICONEXCLAMATION "This is not a valid installation directory."
  Goto errorInDirectory

spaceInDirectory:
  MessageBox MB_OK|MB_ICONEXCLAMATION "The installation path can't contain spaces."
  Goto errorInDirectory
  
directoryToLong:
  MessageBox MB_OK|MB_ICONEXCLAMATION "The installation directory is to long."
  Goto errorInDirectory

errorInDirectory:
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

Function ShowDocumentation
  Exec '$INSTDIR\bin\assistant.exe'
  Exec '$INSTDIR\bin\qtdemo.exe'
FunctionEnd

Function un.onInit
  push $0
  call un.SetAdminVar
  StrCmp $RUNNING_AS_ADMIN "true" Done
  
  ; We are running as a restriced user... make sure it has been installed as one too
  ReadRegStr $0 HKLM "SOFTWARE\trolltech\Versions\${PRODUCT_VERSION}\" "InstallDir"
  StrCmp $0 $INSTDIR 0 Done
  MessageBox MB_OK|MB_ICONSTOP "You do not have the required access rights to uninstall this package."
  Abort

  Done:
  pop $0
FunctionEnd

Section Uninstall
  AddSize 150000
  DetailPrint "Removing start menu shortcuts"

  Delete "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Assistant.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Designer.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Linguist.lnk"

  Delete "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\${PRODUCT_NAME} ${PRODUCT_VERSION} Command Prompt.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Examples and Demos.lnk"

  Delete "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Trolltech.com.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}\Uninstall Qt ${PRODUCT_VERSION}.lnk"

  RMDir "$SMPROGRAMS\${PRODUCT_NAME} by Trolltech v${PRODUCT_VERSION}"
  
  DetailPrint "Removing installation directory..."
  RMDir /r "$INSTDIR"

  #removing the environment variables
  !insertmacro TT_QTENV_UNREGISTER
  
  #removing the qt version for the integration
  DetailPrint "Removing registry entries"
  DeleteRegKey HKCU "SOFTWARE\trolltech\Versions\${PRODUCT_VERSION}\"
  DeleteRegKey HKLM "SOFTWARE\trolltech\Versions\${PRODUCT_VERSION}\"
  
  DeleteRegKey HKCU "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  
  SetAutoClose true
SectionEnd

;sets $RUNNING_AS_ADMIN to "true" if Admin or Power user
!macro SetAdminVar UN
Function ${UN}SetAdminVar
  push $0
  ClearErrors
  UserInfo::GetAccountType
  IfErrors Admin ;It's probably Win95
  pop $0
  StrCmp $0 "Admin" Admin
  StrCmp $0 "Power" Admin

  StrCpy $RUNNING_AS_ADMIN "false"
  goto Done

  Admin:
  StrCpy $RUNNING_AS_ADMIN "true"

  Done:
  pop $0
FunctionEnd
!macroend
!insertmacro SetAdminVar ""
!insertmacro SetAdminVar "un."
