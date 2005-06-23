!define TT_QTKEY_INI_FILE "checkqtlicense.ini"

var LICENSE_KEY
var LICENSEE
var DISPLAY_US_LICENSE
var LICENSE_PRODUCT

LangString LicenseTitle ${LANG_ENGLISH} "Qt License"
LangString LicenseTitleDescription ${LANG_ENGLISH} "Enter your Qt License key."

!macro TT_LICENSE_PAGE_INIT
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${TT_QTKEY_INI_FILE}"
  
  push $0
  push $1
  qtnsisext::GetLicenseInfo
  pop $1 ; Licensee
  pop $0 ; License Key
  !insertmacro MUI_INSTALLOPTIONS_WRITE "${TT_QTKEY_INI_FILE}" "Field 2" "State" "$1"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "${TT_QTKEY_INI_FILE}" "Field 3" "State" "$0"
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
  !insertmacro MUI_INSTALLOPTIONS_READ $LICENSE_KEY "${TT_QTKEY_INI_FILE}" "Field 3" "State"
  push $1
  
  StrLen $1 $LICENSEE
  IntCmp $1 0 wrongLicensee
  
  ClearErrors
  qtnsisext::IsValidLicense $LICENSE_KEY
  IfErrors wrongKey
  pop $1
  strcmp $1 "1" checkForUS
    goto wrongKey

  checkForUS:
    push $1
    qtnsisext::UsesUsLicense $LICENSE_KEY
    pop $1
    strcmp $1 "1" 0 nonUS
      strcpy $DISPLAY_US_LICENSE "1"
      goto checkProduct
  nonUS:
    strcpy $DISPLAY_US_LICENSE "0"

  checkProduct:
    qtnsisext::GetLicenseProduct $LICENSE_KEY
    pop $LICENSE_PRODUCT
    
!ifdef QTEVALUATION
    strcmp "$LICENSE_PRODUCT" "EvaluationUniversal" end
    strcmp "$LICENSE_PRODUCT" "EvaluationDesktop" end
    strcmp "$LICENSE_PRODUCT" "EvaluationDesktopLight" end
    goto wrongKey
!else
    strcmp "$LICENSE_PRODUCT" "CommercialUniversal" end
    strcmp "$LICENSE_PRODUCT" "CommercialDesktop" end
    strcmp "$LICENSE_PRODUCT" "CommercialDesktopLight" end
    goto wrongKey
!endif
  
  wrongLicensee:
    MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "The licensee name is not valid. Do you want to try again?" IDRETRY tryAgain 0
    Quit
  wrongKey:
    MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "The license key you entered is not valid. Do you want to try again?" IDRETRY tryAgain 0
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