; Help Module

!ifdef MODULE_HELP

;------------------------------------------------------------------------------------------------
!macro HELP_INITIALIZE
!ifndef MODULE_HELP_NAME
  !define MODULE_HELP_NAME "Help Integration"
!endif
!ifndef MODULE_HELP_ROOT
  !define MODULE_HELP_ROOT "${INSTALL_ROOT}\vsip\help"
!endif
!ifndef MODULE_HELP_QT_FILE_ROOT
  !define MODULE_HELP_QT_FILE_ROOT "qt"
!endif

!include "includes\system.nsh"
!include "includes\help.nsh"

!macroend

;------------------------------------------------------------------------------------------------

!macro HELP_SECTIONS

Section -PreHelpSection
  ; use default instdir if not set
  strcmp "$HELP_INSTDIR" "" 0 +2
    StrCpy $HELP_INSTDIR "$INSTDIR\help"
SectionEnd

SectionGroup "Help Integration"

Section "Visual Studio 2005" HELP_SEC01
  !insertmacro InstallHelp "$HELP_INSTDIR" ${MODULE_HELP_QT_FILE_ROOT} "8.0"
  WriteRegDWORD SHCTX "SOFTWARE\Trolltech\QtHelp" "${PRODUCT_NAME} ${PRODUCT_VERSION} - 8.0" 1
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "QtHelp80Installed" 1
SectionEnd

Section "Visual Studio 2003" HELP_SEC02
  !insertmacro InstallHelp "$HELP_INSTDIR" ${MODULE_HELP_QT_FILE_ROOT} "7.1"
  WriteRegDWORD SHCTX "SOFTWARE\Trolltech\QtHelp" "${PRODUCT_NAME} ${PRODUCT_VERSION} - 7.1" 1
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "QtHelp71Installed" 1
SectionEnd

Section "Visual Studio 2002" HELP_SEC03
  !insertmacro InstallHelp "$HELP_INSTDIR" ${MODULE_HELP_QT_FILE_ROOT} "7.0"
  WriteRegDWORD SHCTX "SOFTWARE\Trolltech\QtHelp" "${PRODUCT_NAME} ${PRODUCT_VERSION} - 7.0" 1
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "QtHelp70Installed" 1
SectionEnd

SectionGroupEnd

Section -PostHelpSection
  IfFileExists "$HELP_INSTDIR\h2reg.exe" 0 PostHelpSectionFinished
  !insertmacro RegisterHelp "$HELP_INSTDIR" ${MODULE_HELP_QT_FILE_ROOT}
PostHelpSectionFinished:
SectionEnd

!macroend

;------------------------------------------------------------------------------------------------
!macro HELP_DESCRIPTION
!ifdef HELP_SEC01
  !insertmacro MUI_DESCRIPTION_TEXT ${HELP_SEC01} "This installs the Qt Help Integration for Visual Studio 2005"
!endif
!ifdef HELP_SEC02
  !insertmacro MUI_DESCRIPTION_TEXT ${HELP_SEC02} "This installs the Qt Help Integration for Visual Studio 2003"
!endif
!ifdef HELP_SEC03
  !insertmacro MUI_DESCRIPTION_TEXT ${HELP_SEC03} "This installs the Qt Help Integration for Visual Studio 2002"
!endif
!macroend

;------------------------------------------------------------------------------------------------
!macro HELP_STARTUP
  Push $0
  Push $1
  Push $2
  
  StrCmp $RUNNING_AS_ADMIN "false" 0 Module_Help_CheckForInstallation
    SectionSetFlags ${HELP_SEC01} "16"
    SectionSetFlags ${HELP_SEC02} "16"
    SectionSetFlags ${HELP_SEC03} "16"
    Goto Module_Help_Done

  Module_Help_CheckForInstallation:
    ReadRegDWORD $0 SHCTX "SOFTWARE\Trolltech\QtHelp" "${PRODUCT_NAME} ${PRODUCT_VERSION} - 8.0"
    intcmp $0 1 0 +2
      SectionSetFlags ${HELP_SEC01} "16"
      
    ReadRegDWORD $0 SHCTX "SOFTWARE\Trolltech\QtHelp" "${PRODUCT_NAME} ${PRODUCT_VERSION} - 7.1"
    intcmp $0 1 0 +2
      SectionSetFlags ${HELP_SEC02} "16"
      
    ReadRegDWORD $0 SHCTX "SOFTWARE\Trolltech\QtHelp" "${PRODUCT_NAME} ${PRODUCT_VERSION} - 7.0"
    intcmp $0 1 0 +2
      SectionSetFlags ${HELP_SEC03} "16"

    ClearErrors
    Push "8.0"
    Call GetVSInstallationDir
    Pop $2
    IfErrors 0 +2
      SectionSetFlags ${HELP_SEC01} "16"
      
    ClearErrors
    Push "7.1"
    Call GetVSInstallationDir
    Pop $2
    IfErrors 0 +2
      SectionSetFlags ${HELP_SEC02} "16"

    ClearErrors
    Push "7.0"
    Call GetVSInstallationDir
    Pop $2
    IfErrors 0 +2
      SectionSetFlags ${HELP_SEC03} "16"

  Module_Help_Done:
  Pop $2
  Pop $1
  Pop $0
!macroend

;------------------------------------------------------------------------------------------------

!macro HELP_FINISH
!macroend

;------------------------------------------------------------------------------------------------

!macro HELP_UNSTARTUP
  ; use default instdir if not set
  strcmp "$HELP_INSTDIR" "" 0 +2
    StrCpy $HELP_INSTDIR "$INSTDIR\help"
!macroend

;------------------------------------------------------------------------------------------------

!macro HELP_UNINSTALL
Section un."${MODULE_HELP_NAME}"
  push $0
  push $1
  push $2
  push $3
  
  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "QtHelp80Installed"
  ReadRegDWORD $1 SHCTX "$PRODUCT_UNIQUE_KEY" "QtHelp71Installed"
  ReadRegDWORD $2 SHCTX "$PRODUCT_UNIQUE_KEY" "QtHelp70Installed"

  IntOp $3 $0 | $1
  IntOp $3 $3 | $2
  IntCmp $3 0 Module_Help_End
  
  !insertmacro un.RegisterHelp "$HELP_INSTDIR" ${MODULE_HELP_QT_FILE_ROOT}

  IntCmp $0 0 Module_Help_NoVS2005
  !insertmacro un.InstallHelp "$HELP_INSTDIR" ${MODULE_HELP_QT_FILE_ROOT} "8.0"
  DeleteRegValue SHCTX "$PRODUCT_UNIQUE_KEY" "QtHelp80Installed"
  DeleteRegValue SHCTX "SOFTWARE\Trolltech\QtHelp" "${PRODUCT_NAME} ${PRODUCT_VERSION} - 8.0"
  Module_Help_NoVS2005:
  
  IntCmp $1 0 Module_Help_NoVS2003
  !insertmacro un.InstallHelp "$HELP_INSTDIR" ${MODULE_HELP_QT_FILE_ROOT} "7.1"
  DeleteRegValue SHCTX "$PRODUCT_UNIQUE_KEY" "QtHelp71Installed"
  DeleteRegValue SHCTX "SOFTWARE\Trolltech\QtHelp" "${PRODUCT_NAME} ${PRODUCT_VERSION} - 7.1"
  Module_Help_NoVS2003:

  IntCmp $2 0 Module_Help_NoVS2002
  !insertmacro un.InstallHelp "$HELP_INSTDIR" ${MODULE_HELP_QT_FILE_ROOT} "7.0"
  DeleteRegValue SHCTX "$PRODUCT_UNIQUE_KEY" "QtHelp70Installed"
  DeleteRegValue SHCTX "SOFTWARE\Trolltech\QtHelp" "${PRODUCT_NAME} ${PRODUCT_VERSION} - 7.0"
  Module_Help_NoVS2002:

  DeleteRegKey /ifempty SHCTX "SOFTWARE\Trolltech\QtHelp"

  Call un.DeleteH2RegFiles

  Module_Help_End:
  pop $3
  pop $2
  pop $1
  pop $0
SectionEnd
!macroend

;------------------------------------------------------------------------------------------------

!macro HELP_UNFINISH
!macroend

;------------------------------------------------------------------------------------------------

!else
!macro HELP_INITIALIZE
!macroend
!macro HELP_SECTIONS
!macroend
!macro HELP_DESCRIPTION
!macroend
!macro HELP_STARTUP
!macroend
!macro HELP_FINISH
!macroend
!macro HELP_UNSTARTUP
!macroend
!macro HELP_UNINSTALL
!macroend
!macro HELP_UNFINISH
!macroend
!endif
