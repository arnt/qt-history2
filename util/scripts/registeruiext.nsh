!include "MUI.nsh"

!define UI_EXT_INI_FILE "registeruiext.ini"

!define UI_FILE_INTERNAL_DESC "TrolltechDesignerUI"
!define UI_FILE_ICON "ui.ico"
!define UI_FILE_OPEN_DESC "Open with Qt Designer"
!define DESIGNER_DESC "Qt Designer"
!define DESIGNER_CMD "bin\designer.exe $\"%1$\""
!define DESIGNER_CMD_SHORT "designer.exe"


var REGISTER_UI_EXT_STATE

LangString RegisterUIExtTitle ${LANG_ENGLISH} "File Extension"
LangString RegisterUIExtTitleDescription ${LANG_ENGLISH} "Setting up the File Extension"

!macro TT_PAGES_INIT
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${UI_EXT_INI_FILE}"
  
  strcpy $REGISTER_UI_EXT_STATE "1"
!macroend

!macro TT_PAGE_REGISTER_UI_EXT
  Page custom ShowUIExtPage RegisterUIExtension
!macroend

!macro TT_UNREGISTER_UI_EXT
  call un.UnregisterUIExtension
!macroend

Function ShowUIExtPage
  !insertmacro MUI_HEADER_TEXT "$(RegisterUIExtTitle)" "$(RegisterUIExtTitleDescription)"
  strcmp $REGISTER_UI_EXT_STATE "1" 0 checkNo
    !insertmacro MUI_INSTALLOPTIONS_WRITE "${UI_EXT_INI_FILE}" "Field 2" "State" "1"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "${UI_EXT_INI_FILE}" "Field 3" "State" "0"
    goto showPage
  checkNo:
    !insertmacro MUI_INSTALLOPTIONS_WRITE "${UI_EXT_INI_FILE}" "Field 2" "State" "0"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "${UI_EXT_INI_FILE}" "Field 3" "State" "1"
         
  showPage:
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "${UI_EXT_INI_FILE}"
FunctionEnd

Function RegisterUIExtension
  ; todo: where do we get the icon from?
  ;SetOutPath "$INSTDIR"
  ;File "${PATH_TO_ICON}\${UI_FILE_ICON}"
  !insertmacro MUI_INSTALLOPTIONS_READ $REGISTER_UI_EXT_STATE "${UI_EXT_INI_FILE}" "Field 2" "State"
  strcmp $REGISTER_UI_EXT_STATE "1" 0 end
    WriteRegStr "HKCR" "${UI_FILE_INTERNAL_DESC}" "" ""
    WriteRegStr "HKCR" "${UI_FILE_INTERNAL_DESC}\DefaultIcon" "" "$INSTDIR\${UI_FILE_ICON}"
    WriteRegStr "HKCR" "${UI_FILE_INTERNAL_DESC}\shell\${UI_FILE_OPEN_DESC}\command" "" "$INSTDIR\${DESIGNER_CMD}"
    
    WriteRegStr "HKCR" "Applications\${DESIGNER_CMD_SHORT}" "" ""
    WriteRegStr "HKCR" "Applications\${DESIGNER_CMD_SHORT}\shell\" "FriendlyCache" "${DESIGNER_DESC}"
    WriteRegStr "HKCR" "Applications\${DESIGNER_CMD_SHORT}\shell\open\command" "" "$INSTDIR\${DESIGNER_CMD} "
    
    ; Overwrite it silently...
    WriteRegStr "HKCR" ".ui" "" "${UI_FILE_INTERNAL_DESC}"
  end:
FunctionEnd

Function un.UnregisterUIExtension
  push $1
  ReadRegStr $1 "HKCR" ".ui" ""
  strcmp $1 "${UI_FILE_INTERNAL_DESC}" 0 continue
    ; do not delete this key since a subkey openwithlist
    ; or open withprogid may exist
    WriteRegStr "HKCR" ".ui" "" ""
  continue:
  ; be very carefull of what we delete
  DeleteRegValue "HKCR" "Applications\${DESIGNER_CMD_SHORT}\shell\open\command" ""
  DeleteRegKey /ifempty "HKCR" "Applications\${DESIGNER_CMD_SHORT}\shell\open\command"
  DeleteRegKey /ifempty "HKCR" "Applications\${DESIGNER_CMD_SHORT}\shell\open"
  DeleteRegValue "HKCR" "Applications\${DESIGNER_CMD_SHORT}\shell" "FriendlyCache"
  DeleteRegKey /ifempty "HKCR" "Applications\${DESIGNER_CMD_SHORT}\shell"
  DeleteRegKey /ifempty "HKCR" "Applications\${DESIGNER_CMD_SHORT}"
  
  ; just delete it since nobody else is supposed to use it
  DeleteRegKey "HKCR" "${UI_FILE_INTERNAL_DESC}"
  pop $1
FunctionEnd
