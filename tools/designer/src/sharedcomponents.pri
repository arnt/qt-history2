contains(TEMPLATE,lib) {
   CONFIG(debug, debug|release) {
      unix:TARGET = $$member(TARGET, 0)_debug
      else:TARGET = $$member(TARGET, 0)d
   }
}

for(QTSHAREDLIB, $$list($$unique(LIBS))) {
    sharedlib =
    isEqual(QTSHAREDLIB, -lformeditor):sharedlib = -lformeditor
    else:isEqual(QTSHAREDLIB, -lobjectinspector):sharedlib = -lobjectinspector
    else:isEqual(QTSHAREDLIB, -lpropertyeditor):sharedlib = -lpropertyeditor
    else:isEqual(QTSHAREDLIB, -lwidgetbox):sharedlib = -lwidgetbox
    else:isEqual(QTSHAREDLIB, -limagecollection):sharedlib = -limagecollection
    else:isEqual(QTSHAREDLIB, -luilib):sharedlib = -luilib
    else:isEqual(QTSHAREDLIB, -lsignalsloteditor):sharedlib = -lsignalsloteditor
    else:isEqual(QTSHAREDLIB, -lbuddyeditor):sharedlib = -lbuddyeditor
    else:isEqual(QTSHAREDLIB, -lshared):sharedlib = -lshared
    else:isEqual(QTSHAREDLIB, -ltaskmenu):sharedlib = -ltaskmenu
    else:isEqual(QTSHAREDLIB, -lQtDesigner):sharedlib = -lQtDesigner
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

view3d: DEFINES += DESIGNER_VIEW3D
