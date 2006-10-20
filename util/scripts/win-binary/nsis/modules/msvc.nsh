!ifdef MODULE_MSVC

!macro MSVC_INITIALIZE
!include "includes\qtcommon.nsh"
!ifndef MODULE_MSVC_NAME
  !define MODULE_MSVC_NAME "Qt"
!endif
!ifndef MODULE_MSVC_VERSION
  !define MODULE_MSVC_VERSION "${PRODUCT_VERSION}"
!endif
!ifndef MODULE_MSVC_BUILDDIR
  !error "MODULE_MSVC_BUILDDIR not defined!"
!endif
!ifndef MODULE_MSVC_ROOT
  !error "MODULE_MSVC_ROOT not defined!"
!endif
!include "includes\qtenv.nsh"
!macroend

!macro MSVC_SECTIONS
Section "${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION}" MSVC_SEC01
  strcmp "$MSVC_INSTDIR" "" 0 +5
    StrCpy $MSVC_INSTDIR "$INSTDIR\${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION}"
    push $MSVC_INSTDIR
    call MakeQtDirectory
    pop $MSVC_INSTDIR

  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "MSVCInstalled" 1
  
  SetOutPath "$MSVC_INSTDIR"
  SetOverwrite ifnewer
  !insertmacro MODULE_MSVC_INSTALLFILES

  push "$MSVC_INSTDIR"
  call DeleteFloatingLicenseProgram
  
  push "$MSVC_INSTDIR\bin"
  call AddStartmenuApplication
  
  push ${MODULE_MSVC_BUILDDIR}
  push "$MSVC_INSTDIR"
  call PatchPrlFiles
  
  IfFileExists "$MSVC_INSTDIR\.qmake.cache" 0 +5
  push "$MSVC_INSTDIR\.qmake.cache"
  push ${MODULE_MSVC_BUILDDIR}
  push $MSVC_INSTDIR
  call PatchPath

  IfFileExists "$MSVC_INSTDIR\mkspecs\default\qmake.conf" 0 +5
  push "$MSVC_INSTDIR\mkspecs\default\qmake.conf"
  push ${MODULE_MSVC_BUILDDIR}
  push $MSVC_INSTDIR
  call PatchPath

  push $MSVC_INSTDIR
  call PatchCommonBinaryFiles
  
  push $MSVC_INSTDIR
  call PatchLicenseInformation

  call PatchMSVCBinaryFiles

  WriteRegStr SHCTX "SOFTWARE\Trolltech\Common\${MODULE_MSVC_VERSION}\$LICENSE_PRODUCT" "Key" "$LICENSE_KEY"

  push $0
  WriteRegStr HKCU "SOFTWARE\trolltech\Versions\${MODULE_MSVC_VERSION}\" "InstallDir" "$MSVC_INSTDIR"
  ReadRegStr $0 HKCU "SOFTWARE\trolltech\Versions\" "DefaultQtVersion"
  strcmp $0 "" 0 +2
    WriteRegStr HKCU "SOFTWARE\trolltech\Versions\" "DefaultQtVersion" "${MODULE_MSVC_VERSION}"
  pop $0

  StrCmp "$RUNNING_AS_ADMIN" "false" +2
    WriteRegStr HKLM "SOFTWARE\trolltech\Versions\${MODULE_MSVC_VERSION}\" "InstallDir" "$MSVC_INSTDIR"

  IfFileExists "$MSVC_INSTDIR\bin\qmake.exe" 0 noqmake
  push ${INSTALL_COMPILER}
  push $MSVC_INSTDIR
  call MakeQtVarsFile

  CreateShortCut "$SMPROGRAMS\$STARTMENU_STRING\Visual Studio with ${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION}.lnk" "$MSVC_INSTDIR\bin\qtvars.bat" "vsstart"

  push $0
  push $1
  ExpandEnvStrings $1 "%COMSPEC%"
  ${StrStr} $0 "$1" "command.com"
  strcmp $0 "" 0 ComSpec_Command
    CreateShortCut "$SMPROGRAMS\$STARTMENU_STRING\${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION} Command Prompt.lnk" "%COMSPEC%" '/k "$MSVC_INSTDIR\bin\qtvars.bat vsvars"'
    goto ComSpec_End
  ComSpec_Command:
    CreateShortCut "$SMPROGRAMS\$STARTMENU_STRING\${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION} Command Prompt.lnk" "%COMSPEC%" "/e:4096 /k $MSVC_INSTDIR\bin\qtvars.bat vsvars"
  ComSpec_End:
  pop $1
  pop $0
      
  noqmake:
SectionEnd

Function PatchMSVCBinaryFiles
  push $0
  push $1

  FindFirst $0 $1 "$MSVC_INSTDIR\bin\*.pdb"
  loop:
    StrCmp $1 "" done
    DetailPrint "Patching $1..."
!ifdef MODULE_MSVC_VC60
    qtnsisext::PatchVC6Binary "$MSVC_INSTDIR\bin\$1" "${MODULE_MSVC_BUILDDIR}" "$MSVC_INSTDIR"
!else
    qtnsisext::PatchVC7Binary "$MSVC_INSTDIR\bin\$1" "${MODULE_MSVC_BUILDDIR}" "$MSVC_INSTDIR"
!endif
    FindNext $0 $1
    Goto loop
  done:

  IfFileExists "$MSVC_INSTDIR\lib\qtmaind.lib" 0 doneqtmaind
  DetailPrint "Patching qtmaind.lib..."
!ifdef MODULE_MSVC_VC60
  qtnsisext::PatchVC6Binary "$MSVC_INSTDIR\lib\qtmaind.lib" "${MODULE_MSVC_BUILDDIR}" "$MSVC_INSTDIR"
!else
  qtnsisext::PatchVC7Binary "$MSVC_INSTDIR\lib\qtmaind.lib" "${MODULE_MSVC_BUILDDIR}" "$MSVC_INSTDIR"
!endif
  doneqtmaind:

  pop $1
  pop $0
FunctionEnd

Function MSVC_ValidateDirectoryFunc
  push "${MODULE_MSVC_BUILDDIR}"
  push $MSVC_INSTDIR
  call CommonCheckDirectory
FunctionEnd
!macroend

!macro MSVC_DESCRIPTION
  !insertmacro MUI_DESCRIPTION_TEXT ${MSVC_SEC01} "This installs ${MODULE_MSVC_NAME} version ${MODULE_MSVC_VERSION} on your system."
!macroend

!macro MSVC_STARTUP
  !ifndef MODULE_MSVC_NODEFAULT
    SectionSetFlags ${MSVC_SEC01} 17
  !endif
  strcpy $MSVC_INSTDIR "C:\Qt\${MODULE_MSVC_VERSION}"
  push $MSVC_INSTDIR
  call MakeQtDirectory
  pop $MSVC_INSTDIR
!macroend

!macro MSVC_FINISH
!macroend

!macro MSVC_RUN_FUNCTION
  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "MSVCInstalled"
  intcmp $0 1 0 DoneRunFunctionMSVC

  IfFileExists "$MSVC_INSTDIR\bin\qtdemo.exe" 0 +2
    Exec '$MSVC_INSTDIR\bin\qtdemo.exe'
  goto DoneRunFunction ;don't run more applications
  
  DoneRunFunctionMSVC:
!macroend

!macro MSVC_README_FUNCTION
  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "MSVCInstalled"
  intcmp $0 1 0 DoneReadmeFunctionMSVC

  IfFileExists "$MSVC_INSTDIR\bin\assistant.exe" 0 +2
    Exec '$MSVC_INSTDIR\bin\assistant.exe'
  goto DoneReadmeFunction ;don't run more applications

  DoneReadmeFunctionMSVC:
!macroend

!macro MSVC_UNSTARTUP
  strcmp "$MSVC_INSTDIR" "" 0 +5
    StrCpy $MSVC_INSTDIR "$INSTDIR\${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION}"
    push $MSVC_INSTDIR
    call un.MakeQtDirectory
    pop $MSVC_INSTDIR

  !insertmacro ConfirmOnRemove "MSVCInstalled" "- ${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION} in $MSVC_INSTDIR"
!macroend

!macro MSVC_UNINSTALL
Section un."${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION}"
  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "MSVCInstalled"
  intcmp $0 1 0 DoneUnInstallMSVC
  
  DetailPrint "Removing start menu shortcuts"
  call un.RemoveStartmenuApplication
  
  Delete "$SMPROGRAMS\$STARTMENU_STRING\Visual Studio with ${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION}.lnk"
  Delete "$SMPROGRAMS\$STARTMENU_STRING\${MODULE_MSVC_NAME} ${MODULE_MSVC_VERSION} Command Prompt.lnk"

  Delete "$MSVC_INSTDIR\bin\qtvars.bat"
  
  !insertmacro MODULE_MSVC_REMOVE "$MSVC_INSTDIR"
  RMDir $MSVC_INSTDIR ;removes it if empty

  DetailPrint "Removing registry entries"
  DeleteRegKey HKCU "SOFTWARE\trolltech\Versions\${MODULE_MSVC_VERSION}\"
  DeleteRegKey HKLM "SOFTWARE\trolltech\Versions\${MODULE_MSVC_VERSION}\"

  DoneUnInstallMSVC:
SectionEnd
!macroend
!macro MSVC_UNFINISH
!macroend
!else ;MODULE_MSVC
!macro MSVC_INITIALIZE
!macroend
!macro MSVC_SECTIONS
!macroend
!macro MSVC_DESCRIPTION
!macroend
!macro MSVC_STARTUP
!macroend
!macro MSVC_FINISH
!macroend
!macro MSVC_README_FUNCTION
!macroend
!macro MSVC_RUN_FUNCTION
!macroend
!macro MSVC_UNSTARTUP
!macroend
!macro MSVC_UNINSTALL
!macroend
!macro MSVC_UNFINISH
!macroend
!endif ;MODULE_MSVC

