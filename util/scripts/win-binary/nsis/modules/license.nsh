!ifdef MODULE_LICENSECHECK

!macro LICENSECHECK_INITIALIZE
  !include "includes\list.nsh"
  !define TT_QTKEY_INI_FILE "license.ini"
  !define MODULE_LICENSECHECK_PAGE "licensepage.ini"

  LangString LicenseTitle ${LANG_ENGLISH} "Qt License"
  LangString LicenseTitleDescription ${LANG_ENGLISH} "Enter your Qt License key."

  LangString LicensePageTitle ${LANG_ENGLISH} "License Agreement"
  LangString LicensePageDescription ${LANG_ENGLISH} "Please review the license terms before installing $(^Name)."

  Page custom CheckQtLicense ValidateKey
  Page custom ModuleLicensePageEnter ModuleLicensePageExit
  
  !ifdef MODULE_LICENSECHECK_PACKAGEDATE
    !echo "Using package date: ${MODULE_LICENSECHECK_PACKAGEDATE}"
  !endif
!macroend

!macro LICENSECHECK_SECTIONS
  Section -ModuleLicenseCheck
    push $0
    push $1
    push $2
    push $3
    
    qtnsisext::GetLicenseInfo
    pop $1 ; Licensee
    pop $3 ; OldLicenseKey
    pop $0 ; License Key
    
    strcmp "$1" "$LICENSE_KEY" done ;use the .qt-license that already exists

    ; create a new .qt-license file
    ExpandEnvStrings $0 "%HOME%"
    strcmp "$0" "%HOME%" 0 writeLicenseFile
    ExpandEnvStrings $0 "%USERPROFILE%"
    strcmp "$0" "%USERPROFILE%" 0 writeLicenseFile
    ExpandEnvStrings $0 "%HOMEDRIVE%%HOMEPATH%"

    ; if bka exists, write backup to file bkb
    writeLicenseFile:
    IfFileExists "$0\.qt-license" 0 noBackup
    
    strcpy $1 "$0\.qt-license-original"
    IfFileExists "$1" 0 +2
      strcpy $1 "$0\.qt-license-backup"

    IfFileExists "$1" 0 +2
      Delete "$1"
      
    ClearErrors
    CopyFiles /FILESONLY "$0\.qt-license" "$1"
    IfErrors done
    Delete "$0\.qt-license"
    IfErrors done
    
    noBackup:
    qtnsisext::GetLicenseID $LICENSE_KEY
    pop $2

    ClearErrors
    FileOpen $1 "$0\.qt-license" w
    IfErrors done
    FileWrite $1 '# Qt license file (Created by the binary installer)$\r$\n'
    FileWrite $1 'LicenseID="$2"$\r$\n'
    FileWrite $1 'Licensee="$LICENSEE"$\r$\n'
    strcmp "$3" "" +2
      FileWrite $1 "LicenseKey=$3$\r$\n"
    FileWrite $1 "LicenseKeyExt=$LICENSE_KEY$\r$\n"
    FileClose $1

    done:
    pop $3
    pop $2
    pop $1
    pop $0
  SectionEnd

  Function ModuleLicensePageEnter
    push $0
    push $1
    !insertmacro MUI_HEADER_TEXT "$(LicensePageTitle)" "$(LicensePageDescription)"

    GetDlgItem $1 $HWNDPARENT 1
    EnableWindow $1 0
    !insertmacro MUI_INSTALLOPTIONS_WRITE "${MODULE_LICENSECHECK_PAGE}" "Field 4" "State" "0"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "${MODULE_LICENSECHECK_PAGE}" "Field 5" "State" "1"

    !insertmacro MUI_INSTALLOPTIONS_INITDIALOG ${MODULE_LICENSECHECK_PAGE}
    FindWindow $0 "#32770" "" $HWNDPARENT
    GetDlgItem $1 $0 1200
    SetCtlColors $1 0x000000 0xFFFFFF
    qtnsisext::ShowLicenseFile "$1" "$LICENSE_FILE"
    !insertmacro MUI_INSTALLOPTIONS_SHOW

    pop $1
    pop $0
  FunctionEnd
  
  Function ModuleLicensePageExit
    push $0
    push $1
    !insertmacro MUI_INSTALLOPTIONS_READ $0 "${MODULE_LICENSECHECK_PAGE}" "Settings" "State"
    strcmp "$0" "4" +2
    strcmp "$0" "5" 0 NoNotify
      !insertmacro MUI_INSTALLOPTIONS_READ $0 "${MODULE_LICENSECHECK_PAGE}" "Field 4" "State"
      GetDlgItem $1 $HWNDPARENT 1
      strcmp "$0" "1" +3
        EnableWindow $1 0
      goto +2
        EnableWindow $1 1
      pop $1
      pop $0
      abort
    NoNotify:

    pop $1
    pop $0
    call CheckLocalLicenseProduct
  FunctionEnd

  Function CheckQtLicense
    !insertmacro MUI_HEADER_TEXT "$(LicenseTitle)" "$(LicenseTitleDescription)"
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY ${TT_QTKEY_INI_FILE}
  FunctionEnd

  Function ValidateKey
    !insertmacro MUI_INSTALLOPTIONS_READ $LICENSEE "${TT_QTKEY_INI_FILE}" "Field 2" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $LICENSE_KEY "${TT_QTKEY_INI_FILE}" "Field 3" "State"
    push $1
    push $2

    StrLen $1 $LICENSEE
    IntCmp $1 0 wrongLicensee

    ClearErrors
    qtnsisext::IsValidLicense $LICENSE_KEY
    IfErrors wrongKey
    pop $1
    strcmp $1 "0" wrongKey

    !ifdef MODULE_LICENSECHECK_CHECKPRODUCT
      qtnsisext::HasLicenseProduct $LICENSE_KEY "${MODULE_LICENSECHECK_CHECKPRODUCT}"
      pop $1
      strcmp $1 "0" wrongProduct
    !endif

    qtnsisext::GetLicenseProduct $LICENSE_KEY
    pop $LICENSE_PRODUCT
    
    !ifdef MODULE_LICENSECHECK_PRODUCTLIST
      push "$LICENSE_PRODUCT"
      push "${MODULE_LICENSECHECK_PRODUCTLIST}"
      call ItemInList
      pop $1
      strcmp $1 "0" wrongKey
    !endif
    
    strcmp "$LICENSE_PRODUCT" "SupportedEvaluation" 0 +2
      strcpy $LICENSE_PRODUCT "Evaluation"
    strcmp "$LICENSE_PRODUCT" "UnsupportedEvaluation" 0 +2
      strcpy $LICENSE_PRODUCT "Evaluation"
    strcmp "$LICENSE_PRODUCT" "FullSourceEvaluation" 0 +2
      strcpy $LICENSE_PRODUCT "Evaluation"

    !ifdef MODULE_LICENSECHECK_PACKAGEDATE
      strcmp "$LICENSE_PRODUCT" "Evaluation" expiryDateCurrent expiryDatePackage
      expiryDateCurrent:
        qtnsisext::IsValidDate $LICENSE_KEY ""
        pop $1
        strcmp $1 "0" keyExpired
      goto expiryDateEnd
      expiryDatePackage:
        qtnsisext::IsValidDate $LICENSE_KEY "${MODULE_LICENSECHECK_PACKAGEDATE}"
        pop $1
        strcmp $1 "0" keyExpired
      expiryDateEnd:
    !endif

    !ifndef MODULE_LICENSECHECK_LICENSE
      ${StrCase} $2 "-$LICENSE_PRODUCT" "U"
      strcmp $2 "-UNIVERSAL" 0 +2
        strcpy $2 ""
      strcmp $2 "-CONSOLE" 0 +2
        strcpy $2 ""
      strcmp $2 "-DESKTOP" 0 +2
        strcpy $2 ""
      strcmp $2 "-DESKTOPLIGHT" 0 +2
        strcpy $2 ""
      strcpy $LICENSE_FILE "$PLUGINSDIR\.LICENSE$2"
    !else
      strcpy $LICENSE_FILE "$PLUGINSDIR\${MODULE_LICENSECHECK_LICENSE}"
    !endif
    
    qtnsisext::UsesUsLicense $LICENSE_KEY
    pop $2
    strcmp "$2" "0" end
    
    !ifdef MODULE_LICENSECHECK_USLICENSE
      strcpy $LICENSE_FILE "$PLUGINSDIR\${MODULE_LICENSECHECK_USLICENSE}"
    !else
      strcpy $LICENSE_FILE "$LICENSE_FILE-US"
    !endif

    goto end
    wrongLicensee:
      MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "The licensee name is not valid. Do you want to try again?" IDRETRY tryAgain 0
      Quit
    wrongKey:
      MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "The license key you entered is not valid. Do you want to try again?" IDRETRY tryAgain 0
      Quit
    keyExpired:
      MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "The license key you entered has expired. Do you want to try again?" IDRETRY tryAgain 0
      Quit
    wrongProduct:
      MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "The license key you entered does not include ${MODULE_LICENSECHECK_CHECKPRODUCT}. Do you want to try again?" IDRETRY tryAgain 0
      Quit
    tryAgain:
      pop $2
      pop $1
      Abort
    end:
      pop $2
      pop $1
  FunctionEnd
!macroend

!macro LICENSECHECK_STARTUP
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${TT_QTKEY_INI_FILE}"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${MODULE_LICENSECHECK_PAGE}"
  
  SetOutPath "$PLUGINSDIR"
  !ifndef MODULE_LICENSECHECK_LICENSE
    File "${MODULE_LICENSECHECK_LICENSEDIR}\.LICENSE*"
  !else
    File "${MODULE_LICENSECHECK_LICENSEDIR}\${MODULE_LICENSECHECK_LICENSE}"
    !ifdef MODULE_LICENSECHECK_USLICENSE
      File "${MODULE_LICENSECHECK_LICENSEDIR}\${MODULE_LICENSECHECK_USLICENSE}"
    !endif
  !endif

  push $0
  push $1
  push $2
  qtnsisext::GetLicenseInfo
  pop $1 ; Licensee
  pop $2 ; OldLicenseKey
  pop $0 ; License Key
  !insertmacro MUI_INSTALLOPTIONS_WRITE "${TT_QTKEY_INI_FILE}" "Field 2" "State" "$1"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "${TT_QTKEY_INI_FILE}" "Field 3" "State" "$0"
  pop $2
  pop $1
  pop $0
!macroend

!macro LICENSECHECK_FINISH
!macroend
!macro LICENSECHECK_UNSTARTUP
!macroend
!macro LICENSECHECK_UNINSTALL
!macroend
!macro LICENSECHECK_UNFINISH
!macroend
!else ;MODULE_LICENSECHECK
!macro LICENSECHECK_INITIALIZE
  !ifdef LICENSE_FILE
    !define MUI_LICENSEPAGE_RADIOBUTTONS
    !insertmacro MUI_PAGE_LICENSE ${LICENSE_FILE}
  !endif
!macroend
!macro LICENSECHECK_SECTIONS
!macroend
!macro LICENSECHECK_STARTUP
!macroend
!macro LICENSECHECK_FINISH
!macroend
!macro LICENSECHECK_UNSTARTUP
!macroend
!macro LICENSECHECK_UNINSTALL
!macroend
!macro LICENSECHECK_UNFINISH
!macroend
!endif ;MODULE_LICENSECHECK
