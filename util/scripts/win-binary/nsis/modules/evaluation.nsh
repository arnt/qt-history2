!ifdef MODULE_EVALUATION
!macro EVALUATION_INITIALIZE
  !ifndef MODULE_EVALUATION_QTDIR
    !ifdef MODULE_MINGW
      !define MODULE_EVALUATION_QTDIR $MINGW_INSTDIR
    !endif

    !ifdef MODULE_MSVC
      !define MODULE_EVALUATION_QTDIR $MSVC_INSTDIR
    !endif
  !endif
!macroend
!macro EVALUATION_SECTIONS
  Section -ModuleEvaluationSection
    call PatchKeyInBinary
  SectionEnd
  
  Function PatchKeyInBinary
    push $0
    push $1

    DetailPrint "Patching key in core..."
    FindFirst $0 $1 "${MODULE_EVALUATION_QTDIR}\bin\QtCore*.dll"
    StrCmp $1 "" ErrorPatchingCore
    qtnsisext::PatchBinary "${MODULE_EVALUATION_QTDIR}\bin\$1" "qt_qevalkey=" "qt_qevalkey=$LICENSE_KEY"

    FindNext $0 $1
    StrCmp $1 "" ErrorPatchingCore
    qtnsisext::PatchBinary "${MODULE_EVALUATION_QTDIR}\bin\$1" "qt_qevalkey=" "qt_qevalkey=$LICENSE_KEY"

    ErrorPatchingCore:

    DetailPrint "Patching key in gui..."
    FindFirst $0 $1 "${MODULE_EVALUATION_QTDIR}\bin\QtGui*.dll"
    StrCmp $1 "" ErrorPatchingGUI
    qtnsisext::PatchBinary "${MODULE_EVALUATION_QTDIR}\bin\$1" "qt_qevalkey=" "qt_qevalkey=$LICENSE_KEY"

    FindNext $0 $1
    StrCmp $1 "" ErrorPatchingGUI
    qtnsisext::PatchBinary "${MODULE_EVALUATION_QTDIR}\bin\$1" "qt_qevalkey=" "qt_qevalkey=$LICENSE_KEY"

    ErrorPatchingGUI:
    
    CopyFiles /FILESONLY "${MODULE_EVALUATION_QTDIR}\include\Qt\qconfig.h" "${MODULE_EVALUATION_QTDIR}\include\QtCore"

    pop $1
    pop $0
  FunctionEnd
!macroend
!macro EVALUATION_DESCRIPTION
!macroend
!macro EVALUATION_STARTUP
!macroend
!macro EVALUATION_FINISH
!macroend
!macro EVALUATION_UNSTARTUP
!macroend
!macro EVALUATION_UNINSTALL
!macroend
!macro EVALUATION_UNFINISH
!macroend
!else ;MODULE_EVALUATION
!macro EVALUATION_INITIALIZE
!macroend
!macro EVALUATION_SECTIONS
!macroend
!macro EVALUATION_DESCRIPTION
!macroend
!macro EVALUATION_STARTUP
!macroend
!macro EVALUATION_FINISH
!macroend
!macro EVALUATION_UNSTARTUP
!macroend
!macro EVALUATION_UNINSTALL
!macroend
!macro EVALUATION_UNFINISH
!macroend
!endif ;MODULE_EVALUATION

