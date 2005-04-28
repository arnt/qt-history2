!define TT_QTKEY_INI_FILE "checkqtlicense.ini"

var KEY1
var KEY2
var KEY3
var LICENSEE
var DISPLAY_US_LICENSE

LangString LicenseTitle ${LANG_ENGLISH} "Qt License"
LangString LicenseTitleDescription ${LANG_ENGLISH} "Enter your Qt License key."

!macro TT_LICENSE_PAGE_INIT
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${TT_QTKEY_INI_FILE}"
  
  push $0
  push $1
  push $2
  push $3
  qtnsisext::GetLicenseInfo
  pop $3 ; Licensee
  pop $0
  pop $1
  pop $2
  !insertmacro MUI_INSTALLOPTIONS_WRITE "${TT_QTKEY_INI_FILE}" "Field 2" "State" "$3"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "${TT_QTKEY_INI_FILE}" "Field 3" "State" "$0"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "${TT_QTKEY_INI_FILE}" "Field 4" "State" "$1"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "${TT_QTKEY_INI_FILE}" "Field 5" "State" "$2"
  pop $2
  pop $1
  pop $0

  strcpy $DISPLAY_US_LICENSE "1"
!macroend

!macro TT_LICENSE_PAGE_SHOW
  Page custom CheckQtLicense ValidateKey

  !define MUI_LICENSEPAGE_RADIOBUTTONS
  !define MUI_PAGE_CUSTOMFUNCTION_PRE ShowUSLicense
  !insertmacro MUI_PAGE_LICENSE "${PACKAGEDIR}\.LICENSE-US"

  !define MUI_LICENSEPAGE_RADIOBUTTONS
  !define MUI_PAGE_CUSTOMFUNCTION_PRE ShowNonUSLicense
  !insertmacro MUI_PAGE_LICENSE "${PACKAGEDIR}\.LICENSE"
!macroend

Function CheckQtLicense
  !insertmacro MUI_HEADER_TEXT "$(LicenseTitle)" "$(LicenseTitleDescription)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "${TT_QTKEY_INI_FILE}"
FunctionEnd

Function ValidateKey
  !insertmacro MUI_INSTALLOPTIONS_READ $LICENSEE "${TT_QTKEY_INI_FILE}" "Field 2" "State"
  !insertmacro MUI_INSTALLOPTIONS_READ $KEY1 "${TT_QTKEY_INI_FILE}" "Field 3" "State"
  !insertmacro MUI_INSTALLOPTIONS_READ $KEY2 "${TT_QTKEY_INI_FILE}" "Field 4" "State"
  !insertmacro MUI_INSTALLOPTIONS_READ $KEY3 "${TT_QTKEY_INI_FILE}" "Field 5" "State"
  push $1
  
  StrLen $1 $LICENSEE
  IntCmp $1 0 wrongKey
  
  ClearErrors
  qtnsisext::IsValidLicense $KEY1 $KEY2 $KEY3
  IfErrors wrongKey
  pop $1
  strcmp $1 "1" checkForUS
    goto wrongKey

  checkForUS:
    push $1
    qtnsisext::UsesUsLicense $KEY1 $KEY2 $KEY3
    pop $1
    strcmp $1 "1" 0 nonUS
      strcpy $DISPLAY_US_LICENSE "1"
      goto end
  nonUS:
    strcpy $DISPLAY_US_LICENSE "0"
  goto end
  wrongKey:
    MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "The license key you entered is not valid, or you did not enter a licencee name. Do you want to try again?" IDRETRY tryAgain 0
    Quit
  tryAgain:
    pop $1
    Abort
  end:
    pop $1
FunctionEnd

Function ShowUSLicense
  strcmp $DISPLAY_US_LICENSE "1" +2
  Abort
FunctionEnd

Function ShowNonUSLicense
  strcmp $DISPLAY_US_LICENSE "0" +2
  Abort
FunctionEnd