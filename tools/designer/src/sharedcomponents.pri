
for(QTSHAREDLIB, $$list($$unique(LIBS))) {
    sharedlib =
         isEqual(QTSHAREDLIB, -lformeditor):sharedlib = -lformeditor
    else:isEqual(QTSHAREDLIB, -lobjectinspector):sharedlib = -lobjectinspector
    else:isEqual(QTSHAREDLIB, -lpropertyeditor):sharedlib = -lpropertyeditor
    else:isEqual(QTSHAREDLIB, -lwidgetbox):sharedlib = -lwidgetbox
    else:isEqual(QTSHAREDLIB, -lsignalsloteditor):sharedlib = -lsignalsloteditor
    else:isEqual(QTSHAREDLIB, -ltabordereditor):sharedlib = -ltabordereditor
    else:isEqual(QTSHAREDLIB, -lresourceeditor):sharedlib = -lresourceeditor
    else:isEqual(QTSHAREDLIB, -lbuddyeditor):sharedlib = -lbuddyeditor
    else:isEqual(QTSHAREDLIB, -ltaskmenu):sharedlib = -ltaskmenu
    else:isEqual(QTSHAREDLIB, -lQtDesigner):sharedlib = -lQtDesigner
    else:isEqual(QTSHAREDLIB, -lQtDesignerCompnents):sharedlib = -lQtDesignerComponents
    else:isEqual(QTSHAREDLIB, -lQtOpenGL):sharedlib = -lQtOpenGL

    !isEmpty(sharedlib) {
        LIBS -= $${sharedlib}
        CONFIG(debug, debug|release) {
            unix: LIBS += $${sharedlib}_debug
            else: LIBS += $${sharedlib}d
        } else {
            LIBS += $${sharedlib}
        }
    }
}

