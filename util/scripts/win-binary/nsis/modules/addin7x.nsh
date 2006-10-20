!ifdef MODULE_ADDIN7X

;------------------------------------------------------------------------------------------------
!macro ADDIN7X_INITIALIZE
!ifndef MODULE_ADDIN7X_NAME
  !define MODULE_ADDIN7X_NAME "QMsNet Add-In"
!endif
!ifndef MODULE_ADDIN7X_VERSION
  !define MODULE_ADDIN7X_VERSION ${PRODUCT_VERSION}
!endif
!ifndef MODULE_ADDIN7X_ROOT
  !define MODULE_ADDIN7X_ROOT "${INSTALL_ROOT}\addin7x"
!endif
!ifndef MODULE_VSIP_ROOT
  !error "MODULE_VSIP_ROOT must be in the .conf file..."
!endif

!include "includes\system.nsh"
!include "includes\templates.nsh"
!macroend ;ADDIN7X_INITIALIZE

;------------------------------------------------------------------------------------------------
!macro ADDIN7X_SECTIONS

SectionGroup "${MODULE_ADDIN7X_NAME}"
!ifndef MODULE_ADDIN7X_NO2005
Section "Visual Studio 2005" ADDIN7X_SEC01
  strcpy $VS_VERSION "2005"
  strcpy $VS_VERSION_SHORT "8.0"

  SetOutPath "$ADDIN_INSTDIR\$VS_VERSION_SHORT"
  File "${MODULE_ADDIN7X_ROOT}\src\bin2005\QMsNet2005.dll"

  SetOutPath "$ADDIN_INSTDIR\$VS_VERSION_SHORT"
  File "${MODULE_ADDIN7X_ROOT}\src\bin2005\QtProjectLib.dll"

  SetOutPath "$ADDIN_INSTDIR\$VS_VERSION_SHORT"
  File "${MODULE_ADDIN7X_ROOT}\src\bin2005\QtProjectEngineLib.dll"

  call RegisterAddin
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "QMsNet2005" 1

  push "$ADDIN_INSTDIR"
  call InstallProjectTemplates

  ; Qt3 wizard
  strcpy $ProjectInstDir "$ADDIN_INSTDIR\projects (Qt3)\$VS_VERSION_SHORT"
  strcpy $WizardInstDir "$ADDIN_INSTDIR\wizards"
  !insertmacro InstallProjectTemplate "${MODULE_ADDIN7X_ROOT}\Projects\Qt3ApplicationProject" "Qt3ApplicationProject"
SectionEnd
!endif

!ifndef MODULE_ADDIN7X_NO2003
Section "Visual Studio 2003" ADDIN7X_SEC02
  strcpy $VS_VERSION "2003"
  strcpy $VS_VERSION_SHORT "7.1"

  SetOutPath "$ADDIN_INSTDIR\$VS_VERSION_SHORT"
  File "${MODULE_ADDIN7X_ROOT}\src\bin2003\QMsNet2003.dll"

  SetOutPath "$ADDIN_INSTDIR\$VS_VERSION_SHORT"
  File "${MODULE_ADDIN7X_ROOT}\src\bin2003\QtProjectLib.dll"

  SetOutPath "$ADDIN_INSTDIR\$VS_VERSION_SHORT"
  File "${MODULE_ADDIN7X_ROOT}\src\bin2003\QtProjectEngineLib.dll"

  call RegisterAddin
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "QMsNet2003" 1

  push "$ADDIN_INSTDIR"
  call InstallProjectTemplates

  ; Qt3 wizard
  strcpy $ProjectInstDir "$ADDIN_INSTDIR\projects (Qt3)\$VS_VERSION_SHORT"
  strcpy $WizardInstDir "$ADDIN_INSTDIR\wizards"
  !insertmacro InstallProjectTemplate "${MODULE_ADDIN7X_ROOT}\Projects\Qt3ApplicationProject" "Qt3ApplicationProject"
SectionEnd
!endif

!ifndef MODULE_ADDIN7X_NO2002
Section "Visual Studio 2002" ADDIN7X_SEC03
  strcpy $VS_VERSION "2002"
  strcpy $VS_VERSION_SHORT "7.0"

  SetOutPath "$ADDIN_INSTDIR\$VS_VERSION_SHORT"
  File "${MODULE_ADDIN7X_ROOT}\src\bin2002\QMsNet2002.dll"

  SetOutPath "$ADDIN_INSTDIR\$VS_VERSION_SHORT"
  File "${MODULE_ADDIN7X_ROOT}\src\bin2002\QtProjectLib.dll"

  SetOutPath "$ADDIN_INSTDIR\$VS_VERSION_SHORT"
  File "${MODULE_ADDIN7X_ROOT}\src\bin2002\QtProjectEngineLib.dll"

  call RegisterAddin
  WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "QMsNet2002" 1

  push "$ADDIN_INSTDIR"
  call InstallProjectTemplates

  ; Qt3 wizard
  strcpy $ProjectInstDir "$ADDIN_INSTDIR\projects (Qt3)\$VS_VERSION_SHORT"
  strcpy $WizardInstDir "$ADDIN_INSTDIR\wizards"
  !insertmacro InstallProjectTemplate "${MODULE_ADDIN7X_ROOT}\Projects\Qt3ApplicationProject" "Qt3ApplicationProject"
SectionEnd
!endif

Section -AddinCommonFiles
  ; make sure one of the .net addins is installed
  IfFileExists "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QMsNet2002.dll" installCommon
  IfFileExists "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QMsNet2003.dll" installCommon
  IfFileExists "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QMsNet2005.dll" installCommon done
  
  installCommon:
  SetOutPath "$ADDIN_INSTDIR\1033"
  File "${MODULE_ADDIN7X_ROOT}\src\bin\1033\QMsNetResources.dll"

  push $ADDIN_INSTDIR
  call InstallItemTemplates

  SetOutPath "$ADDIN_INSTDIR\qt4items"
  File "${MODULE_ADDIN7X_ROOT}\Items\Qt4Files.vsdir"

  SetOutPath "$ADDIN_INSTDIR\qt3items"
  File "${MODULE_ADDIN7X_ROOT}\Items\Qt3Files.vsdir"
  File "${MODULE_ADDIN7X_ROOT}\Items\newuifile.ui"
  File "${MODULE_ADDIN7X_ROOT}\Items\newuifile.ico"

  SetOutPath "$ADDIN_INSTDIR"
  File "${MODULE_ADDIN7X_ROOT}\src\bin\qrcedit.exe"
  File "${MODULE_ADDIN7X_ROOT}\src\bin\msvcr70.dll"
  File "${MODULE_ADDIN7X_ROOT}\src\bin\msvcp70.dll"
  File "${MODULE_ADDIN7X_ROOT}\doc\usage.rtf"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_STRING\QMsNet Usage.lnk" "$ADDIN_INSTDIR\usage.rtf"
  
  done:
SectionEnd

SectionGroupEnd

Function RegisterAddin
  push $2
  push $3

  push "$VS_VERSION_SHORT"
  call IsDotNETInstalled
  Pop $3
  strcpy $2 "$3\regasm.exe"

  ClearErrors
  DetailPrint "Registering ${MODULE_ADDIN7X_NAME}."
  nsExec::ExecToLog '"$2" /codebase "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QMsNet$VS_VERSION.dll"'
  pop $3
  strcmp "$3" "error" 0 RegAddInDone
    MessageBox MB_OK "Can not register QMsNet$VS_VERSION.dll!"
    MessageBox MB_OK '"$2" /codebase "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QMsNet$VS_VERSION.dll"'
  RegAddInDone:

  ClearErrors
  DetailPrint "Registering the Qt Wizard Engine."
  nsExec::ExecToLog '"$2" /codebase "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QtProjectEngineLib.dll"'
  pop $3
  strcmp "$3" "error" 0 RegEngineDone
    MessageBox MB_OK "Can not register QtProjectEngineLib.dll!"
    MessageBox MB_OK '"$2" /codebase "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QtProjectEngineLib.dll"'
  RegEngineDone:

  DeleteRegValue HKLM "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\PreloadAddinState" "QMsNet$VS_VERSION"
  DeleteRegValue HKCU "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\PreloadAddinState" "QMsNet$VS_VERSION"
  ClearErrors

  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "Description" "Qt Visual Studio Addin that simplifies application development with Qt."
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "SatelliteDllName" "QMsNetResources.dll"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "SatelliteDllPath" "$ADDIN_INSTDIR"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "AboutBoxDetails" "For more information about Trolltech, see the Trolltech website at http://www.trolltech.com. For customer support, send an email to support@trolltech.com. Copyright(c) 2006 Trolltech."
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "FriendlyName" "Qt Visual Studio Addin"

  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "CommandLineSafe" 0
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "LoadBehavior" 1
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "CommandPreload" 1
  
  WriteRegBin SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "AboutBoxIcon" \
  0000010002002020100000000000e80200002600\
  00001010100000000000280100000e03000028000000200000\
  00400000000100040000000000800200000000000000000000\
  00000000000000000000000000008000008000000080800080\
  0000008000800080800000c0c0c000808080000000ff0000ff\
  000000ffff00ff000000ff00ff00ffff0000ffffff00000000\
  00000000000000000000000000000000000000000000000000\
  00000000000000000000000000000000aba000000000000000\
  00000000000000bab00000000000000000000000000000aba0\
  0000000000000000000000f00000bab0000000000000888888\
  8888f0abababababa0000000008f77777777f0bababababab0\
  000000008f77777777f0abababababa0000000008f77777777\
  f00000bab00000000000008f77777777fffff0aba000000088\
  80008f777777777777f0bab00000887788008f777777777777\
  f0aba000008ff770778f777777777777f0000000008fff7808\
  8f777777777777ffffff000008ff70008f7777777777777777\
  800000008880008f7777777777777777800000000000008f77\
  77777777777777800000000000008f77777777777777778000\
  00000000008f7777777777777777800000000000008fffffff\
  ffffffffff8000000000000088888888888888888880000000\
  00000000000870000008700000000000000000000008f00000\
  08f00000000000000000000000800000008000000000000000\
  00000008080000080800000000000000000008777880087778\
  80000000000000000008ff778008ff77800000000000000000\
  08fff78008fff7800000000000000000008ff700008ff70000\
  00000000000000000777000007770000000000000000000000\
  000000000000000000fffffffffffffe0ffffffe0ffffffe0f\
  fffffe0fff000000ff000000ff000000ff000000ff000000c7\
  00000f8300000f0000000f0000000f0000000f8300000fc700\
  000fff00000fff00000fff00000fff00000fff00000ffff8f8\
  fffff8f8fffff8f8fffff0707fffe0203fffe0203fffe0203f\
  fff0707ffff8f8ffffffffff28000000100000002000000001\
  00040000000000c00000000000000000000000000000000000\
  00000000000000008000008000000080800080000000800080\
  0080800000c0c0c000808080000000ff0000ff000000ffff00\
  ff000000ff00ff00ffff0000ffffff00000000000000000000\
  00000000000000000000000000a000000000000000b0000000\
  0000f0ababa000008f88f000b00000008f77fff0a00070708f\
  7777f000000f008f7777ffff0070708f777777800000008fff\
  ffff8000000088888888800000000000000000000000000707\
  00000000000000f00000000000000707000000ffff0000ffe3\
  0000ffe30000ff800000f0000000f0000000f0030000100300\
  000003000010030000f0030000f0030000ff7f0000fe3f0000\
  fe3f0000fe3f0000

  strcmp $VS_VERSION "2002" Version2002
  strcmp $VS_VERSION "2005" Version2005
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2003_GUID}\/1" "" "Qt4 Projects"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2003_GUID}\/1" "SortPriority" 16
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2003_GUID}\/1" "TemplatesDir" "$ADDIN_INSTDIR\projects\$VS_VERSION_SHORT"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2003_GUID}\/2" "" "Qt3 Projects"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2003_GUID}\/2" "SortPriority" 16
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2003_GUID}\/2" "TemplatesDir" "$ADDIN_INSTDIR\projects (Qt3)\$VS_VERSION_SHORT"

  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2003_GUID}\/1" "" "Qt4 Project Items"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2003_GUID}\/1" "TemplatesDir" "$ADDIN_INSTDIR\qt4items"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2003_GUID}\/1" "SortPriority" 16
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2003_GUID}\/2" "" "Qt3 Project Items"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2003_GUID}\/2" "TemplatesDir" "$ADDIN_INSTDIR\qt3items"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2003_GUID}\/2" "SortPriority" 16

  goto Done
  Version2002:
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2002_GUID}\/1" "" "Qt4 Projects"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2002_GUID}\/1" "SortPriority" 16
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2002_GUID}\/1" "TemplatesDir" "$ADDIN_INSTDIR\projects\$VS_VERSION_SHORT"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2002_GUID}\/2" "" "Qt3 Projects"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2002_GUID}\/2" "SortPriority" 16
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2002_GUID}\/2" "TemplatesDir" "$ADDIN_INSTDIR\projects (Qt3)\$VS_VERSION_SHORT"

  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2002_GUID}\/1" "" "Qt4 Project Items"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2002_GUID}\/1" "TemplatesDir" "$ADDIN_INSTDIR\qt4items"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2002_GUID}\/1" "SortPriority" 16
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2002_GUID}\/2" "" "Qt3 Project Items"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2002_GUID}\/2" "TemplatesDir" "$ADDIN_INSTDIR\qt3items"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2002_GUID}\/2" "SortPriority" 16

  goto Done
  Version2005:
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2005_GUID}\/1" "" "Qt4 Projects"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2005_GUID}\/1" "SortPriority" 16
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2005_GUID}\/1" "TemplatesDir" "$ADDIN_INSTDIR\projects\$VS_VERSION_SHORT"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2005_GUID}\/2" "" "Qt3 Projects"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2005_GUID}\/2" "SortPriority" 16
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\NewProjectTemplates\TemplateDirs\${QMSNET2005_GUID}\/2" "TemplatesDir" "$ADDIN_INSTDIR\projects (Qt3)\$VS_VERSION_SHORT"

  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2005_GUID}\/1" "" "Qt4 Project Items"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2005_GUID}\/1" "TemplatesDir" "$ADDIN_INSTDIR\qt4items"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2005_GUID}\/1" "SortPriority" 16
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2005_GUID}\/2" "" "Qt3 Project Items"
  WriteRegStr SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2005_GUID}\/2" "TemplatesDir" "$ADDIN_INSTDIR\qt3items"
  WriteRegDWORD SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2005_GUID}\/2" "SortPriority" 16
  Done:

  pop $3
  pop $2
FunctionEnd

Function un.RegisterAddin
  push $2
  push $3

  push "$VS_VERSION_SHORT"
  call un.IsDotNETInstalled
  Pop $3
  strcpy $2 "$3\regasm.exe"

  ClearErrors
  IfFileExists "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QtProjectEngineLib.dll" 0 UnRegEngineDone
  nsExec::ExecToLog '"$2" /unregister "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QtProjectEngineLib.dll"'
  pop $3
  strcmp "$3" "error" 0 UnRegEngineDone
    MessageBox MB_OK "Can not unregister QtProjectEngineLib.dll!"
  UnRegEngineDone:

  ClearErrors
  IfFileExists "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QMsNet$VS_VERSION.dll" 0 UnRegAddInDone
  nsExec::ExecToLog '"$2" /unregister "$ADDIN_INSTDIR\$VS_VERSION_SHORT\QMsNet$VS_VERSION.dll"'
  pop $3
  strcmp "$3" "error" 0 UnRegAddInDone
    MessageBox MB_OK "Can not unregister QMsNet$VS_VERSION.dll!"
  UnRegAddInDone:

  DeleteRegValue SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "Description"
  DeleteRegValue SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "SatelliteDllName"
  DeleteRegValue SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "SatelliteDllPath"
  DeleteRegValue SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "AboutBoxDetails"
  DeleteRegValue SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "FriendlyName"
  DeleteRegValue SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "CommandLineSafe"
  DeleteRegValue SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "LoadBehavior"
  DeleteRegValue SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "CommandPreload"
  DeleteRegValue SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION" "AboutBoxIcon"
  DeleteRegKey /ifempty SHCTX "SOFTWARE\Microsoft\VisualStudio\$VS_VERSION_SHORT\AddIns\QMsNet$VS_VERSION"
 
  pop $3
  pop $2
FunctionEnd

Function GetVSVersion
  push $0

  ReadRegStr $0 HKLM "Software\Microsoft\VisualStudio\8.0\Setup\VS" "ProductDir"
  StrCmp $0 "" 0 foundVS2005 ; found msvc.net 2005

  ReadRegStr $0 HKLM "Software\Microsoft\VisualStudio\7.1\Setup\VS" "ProductDir"
  StrCmp $0 "" 0 foundVS2003 ; found msvc.net 2003

  ReadRegStr $0 HKLM "Software\Microsoft\VisualStudio\7.0\Setup\VS" "ProductDir"
  StrCmp $0 "" 0 foundVS2002 ; found msvc.net 2002

  push "" ;empty string if not found
  goto done

  foundVS2005:
  push "2005"
  goto done

  foundVS2003:
  push "2003"
  goto done

  foundVS2002:
  push "2002"
  goto done

  done:
  exch
  pop $0
FunctionEnd
!macroend ;ADDIN7X_SECTIONS

;------------------------------------------------------------------------------------------------
!macro ADDIN7X_DESCRIPTION
!ifdef ADDIN7X_SEC01
  !insertmacro MUI_DESCRIPTION_TEXT ${ADDIN7X_SEC01} "This installs the QMsNet Add-in ${MODULE_ADDIN7X_VERSION} for Visual Studio 2005"
!endif
!ifdef ADDIN7X_SEC02
  !insertmacro MUI_DESCRIPTION_TEXT ${ADDIN7X_SEC02} "This installs the QMsNet Add-in ${MODULE_ADDIN7X_VERSION} for Visual Studio 2003"
!endif
!ifdef ADDIN7X_SEC03
  !insertmacro MUI_DESCRIPTION_TEXT ${ADDIN7X_SEC03} "This installs the QMsNet Add-in ${MODULE_ADDIN7X_VERSION} for Visual Studio 2002"
!endif
!macroend

;------------------------------------------------------------------------------------------------
!macro ADDIN7X_STARTUP
  push $0
  push $1

!ifdef ADDIN7X_SEC01
  SectionSetFlags ${ADDIN7X_SEC01} 0
!endif
!ifdef ADDIN7X_SEC02
  SectionSetFlags ${ADDIN7X_SEC02} 0
!endif
!ifdef ADDIN7X_SEC03
  SectionSetFlags ${ADDIN7X_SEC03} 0
!endif

!ifdef ADDIN7X_SEC03
  ReadRegStr $0 HKLM "Software\Microsoft\VisualStudio\7.0" "InstallDir"
  strcmp $0 "" +3
    SectionSetFlags ${ADDIN7X_SEC03} 1
  goto +2
    SectionSetFlags ${ADDIN7X_SEC03} 16
!endif

!ifdef ADDIN7X_SEC02
  ReadRegStr $0 HKLM "Software\Microsoft\VisualStudio\7.1" "InstallDir"
  strcmp $0 "" +3
    SectionSetFlags ${ADDIN7X_SEC02} 1
  goto +2
    SectionSetFlags ${ADDIN7X_SEC02} 16
!endif

!ifdef ADDIN7X_SEC01
  ReadRegStr $0 HKLM "Software\Microsoft\VisualStudio\8.0" "InstallDir"
  strcmp $0 "" +3
    SectionSetFlags ${ADDIN7X_SEC01} 1
  goto +2
    SectionSetFlags ${ADDIN7X_SEC01} 16
!endif

  strcpy $ADDIN_INSTDIR "$PROGRAMFILES\Trolltech\QMsAddin"

  Push "7.1"
  Call IsIntegrationInstalled
  Pop $0
  IntCmp $0 1 Addin7x_Warn

  Push "8.0"
  Call IsIntegrationInstalled
  Pop $0
  IntCmp $0 1 Addin7x_Warn
  
  Goto Addin7x_Done
  Addin7x_Warn:
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer has detected that a version of the Qt Visual Studio Integration is installed.$\r$\nThe Add-In's conflict with the integration."
  Addin7x_Done:
  
  StrCpy $1 ""
  Push "7.0"
  Call IsQMsNetInstalled
  Pop $0
  IntCmp $0 0 +2
    StrCpy $1 "$1$\r$\n - Visual Studio 2002"

  Push "7.1"
  Call IsQMsNetInstalled
  Pop $0
  IntCmp $0 0 +2
    StrCpy $1 "$1$\r$\n - Visual Studio 2003"

  Push "8.0"
  Call IsQMsNetInstalled
  Pop $0
  IntCmp $0 0 +2
    StrCpy $1 "$1$\r$\n - Visual Studio 2005"
    
  StrCmp $1 "" +2
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer has detected that a the following versions of the Add-In are already installed:$1$\r$\nUninstall the previous versions before you reinstall them."

  pop $1
  pop $0
!macroend ;ADDIN7X_STATUP

;------------------------------------------------------------------------------------------------
!macro ADDIN7X_FINISH
!macroend

!macro ADDIN7X_UNSTARTUP
  !insertmacro ConfirmOnRemove "QMsNet2005" "$ADDIN_INSTDIR"
  !insertmacro ConfirmOnRemove "QMsNet2003" "$ADDIN_INSTDIR"
  !insertmacro ConfirmOnRemove "QMsNet2002" "$ADDIN_INSTDIR"
!macroend

;------------------------------------------------------------------------------------------------
!macro ADDIN7X_UNINSTALL
  push $0
  
  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "QMsNet2005"
  intcmp $0 1 0 DoneUnInstall2005
    WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "QMsNet2005" 0
    strcpy $VS_VERSION "2005"
    strcpy $VS_VERSION_SHORT "8.0"
    call un.RegisterAddin
    Delete "$ADDIN_INSTDIR\8.0\QtProjectLib.dll"
    Delete "$ADDIN_INSTDIR\8.0\QtProjectEngineLib.dll"
    Delete "$ADDIN_INSTDIR\8.0\QMsNet2005.dll"
    RmDir "$ADDIN_INSTDIR\8.0"
    DeleteRegKey SHCTX "SOFTWARE\Microsoft\VisualStudio\8.0\NewProjectTemplates\TemplateDirs\${QMSNET2005_GUID}"
    DeleteRegKey SHCTX "SOFTWARE\Microsoft\VisualStudio\8.0\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2005_GUID}"

    push "$ADDIN_INSTDIR"
    call un.InstallProjectTemplates
    strcpy $ProjectInstDir "$ADDIN_INSTDIR\projects (Qt3)\$VS_VERSION_SHORT"
    strcpy $WizardInstDir "$ADDIN_INSTDIR\wizards"
    !insertmacro UnInstallProjectTemplate "Qt3ApplicationProject"
    RmDir "$ProjectInstDir"
  DoneUnInstall2005:

  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "QMsNet2002"
  intcmp $0 1 0 DoneUnInstall2002
    WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "QMsNet2002" 0
    strcpy $VS_VERSION "2002"
    strcpy $VS_VERSION_SHORT "7.0"
    call un.RegisterAddin
    Delete "$ADDIN_INSTDIR\7.0\QtProjectLib.dll"
    Delete "$ADDIN_INSTDIR\7.0\QtProjectEngineLib.dll"
    Delete "$ADDIN_INSTDIR\7.0\QMsNet2002.dll"
    RmDir "$ADDIN_INSTDIR\7.0"
    DeleteRegKey SHCTX "SOFTWARE\Microsoft\VisualStudio\7.0\NewProjectTemplates\TemplateDirs\${QMSNET2002_GUID}"
    DeleteRegKey SHCTX "SOFTWARE\Microsoft\VisualStudio\7.0\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2002_GUID}"

    push "$ADDIN_INSTDIR"
    call un.InstallProjectTemplates
    strcpy $ProjectInstDir "$ADDIN_INSTDIR\projects (Qt3)\$VS_VERSION_SHORT"
    strcpy $WizardInstDir "$ADDIN_INSTDIR\wizards"
    !insertmacro UnInstallProjectTemplate "Qt3ApplicationProject"
    RmDir "$ProjectInstDir"
  DoneUnInstall2002:

  ReadRegDWORD $0 SHCTX "$PRODUCT_UNIQUE_KEY" "QMsNet2003"
  intcmp $0 1 0 DoneUnInstall2003
    WriteRegDWORD SHCTX "$PRODUCT_UNIQUE_KEY" "QMsNet2003" 0
    strcpy $VS_VERSION "2003"
    strcpy $VS_VERSION_SHORT "7.1"
    call un.RegisterAddin
    Delete "$ADDIN_INSTDIR\7.1\QtProjectLib.dll"
    Delete "$ADDIN_INSTDIR\7.1\QtProjectEngineLib.dll"
    Delete "$ADDIN_INSTDIR\7.1\QMsNet2003.dll"
    RmDir "$ADDIN_INSTDIR\7.1"
    DeleteRegKey SHCTX "SOFTWARE\Microsoft\VisualStudio\7.1\NewProjectTemplates\TemplateDirs\${QMSNET2003_GUID}"
    DeleteRegKey SHCTX "SOFTWARE\Microsoft\VisualStudio\7.1\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\AddItemTemplates\TemplateDirs\${QMSNET2003_GUID}"

    push "$ADDIN_INSTDIR"
    call un.InstallProjectTemplates
    strcpy $ProjectInstDir "$ADDIN_INSTDIR\projects (Qt3)\$VS_VERSION_SHORT"
    strcpy $WizardInstDir "$ADDIN_INSTDIR\wizards"
    !insertmacro UnInstallProjectTemplate "Qt3ApplicationProject"
    RmDir "$ProjectInstDir"
  DoneUnInstall2003:

  strcmp $VS_VERSION "2002" DoUninstallAddinCommon
  strcmp $VS_VERSION "2003" DoUninstallAddinCommon
  strcmp $VS_VERSION "2005" DoUninstallAddinCommon DoneUninstallAddinCommon

  DoUninstallAddinCommon:
  Delete "$ADDIN_INSTDIR\qrcedit.exe"
  Delete "$ADDIN_INSTDIR\msvcr70.dll"
  Delete "$ADDIN_INSTDIR\msvcp70.dll"
  Delete "$ADDIN_INSTDIR\1033\QMsNetResources.dll"
  RmDir "$ADDIN_INSTDIR\1033"

  RmDir "$ADDIN_INSTDIR\projects"
  RmDir "$ADDIN_INSTDIR\projects (Qt3)"
  RmDir "$ADDIN_INSTDIR\wizards"

  Delete "$ADDIN_INSTDIR\qt4items\Qt4Files.vsdir"
  push $ADDIN_INSTDIR
  call un.InstallItemTemplates

  Delete "$ADDIN_INSTDIR\qt3items\Qt3Files.vsdir"
  Delete "$ADDIN_INSTDIR\qt3items\newuifile.ui"
  Delete "$ADDIN_INSTDIR\qt3items\newuifile.ico"
  RmDir "$ADDIN_INSTDIR\qt3items"

  Delete "$ADDIN_INSTDIR\usage.rtf"
  Delete "$SMPROGRAMS\$STARTMENU_STRING\QMsNet Usage.lnk"

  RmDir "$ADDIN_INSTDIR"
  
  DoneUninstallAddinCommon:
  pop $0
!macroend ;ADDIN7X_UNINSTALL

!macro ADDIN7X_UNFINISH
!macroend

!else ;MODULE_ADDIN7X
!macro ADDIN7X_INITIALIZE
!macroend
!macro ADDIN7X_SECTIONS
!macroend
!macro ADDIN7X_DESCRIPTION
!macroend
!macro ADDIN7X_STARTUP
!macroend
!macro ADDIN7X_FINISH
!macroend
!macro ADDIN7X_UNSTARTUP
!macroend
!macro ADDIN7X_UNINSTALL
!macroend
!macro ADDIN7X_UNFINISH
!macroend
!endif ;MODULE_ADDIN7X

