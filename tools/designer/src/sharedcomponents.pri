contains(QT_PRODUCT, OpenSource.*):DEFINES *= QT_OPENSOURCE

for(QTSHAREDLIB, $$list($$unique(LIBS))) {
    QT_SHARED_LIB_NAME =
         isEqual(QTSHAREDLIB, -lformeditor):QT_SHARED_LIB_NAME = formeditor
    else:isEqual(QTSHAREDLIB, -lobjectinspector):QT_SHARED_LIB_NAME = objectinspector
    else:isEqual(QTSHAREDLIB, -lpropertyeditor):QT_SHARED_LIB_NAME = propertyeditor
    else:isEqual(QTSHAREDLIB, -lwidgetbox):QT_SHARED_LIB_NAME = widgetbox
    else:isEqual(QTSHAREDLIB, -lsignalsloteditor):QT_SHARED_LIB_NAME = signalsloteditor
    else:isEqual(QTSHAREDLIB, -ltabordereditor):QT_SHARED_LIB_NAME = tabordereditor
    else:isEqual(QTSHAREDLIB, -lresourceeditor):QT_SHARED_LIB_NAME = resourceeditor
    else:isEqual(QTSHAREDLIB, -lbuddyeditor):QT_SHARED_LIB_NAME = buddyeditor
    else:isEqual(QTSHAREDLIB, -ltaskmenu):QT_SHARED_LIB_NAME = taskmenu
    else:isEqual(QTSHAREDLIB, -lQtDesigner):QT_SHARED_LIB_NAME = QtDesigner
    else:isEqual(QTSHAREDLIB, -lQtDesignerComponents):QT_SHARED_LIB_NAME = QtDesignerComponents
    else:isEqual(QTSHAREDLIB, -lQtOpenGL):QT_SHARED_LIB_NAME = QtOpenGL

    !isEmpty(QT_SHARED_LIB_NAME) {
        LIBS -= -l$${QT_SHARED_LIB_NAME}

	QT_SHARED_LINKAGE =
	mac {
	   CONFIG(qt_framework, qt_framework|qt_no_framework) { #forced
	         QMAKE_CFLAGS *= -F$${QMAKE_LIBDIR_QT}
		 QMAKE_CXXFLAGS *= -F$${QMAKE_LIBDIR_QT}
		 QMAKE_LIBDIR_FLAGS *= -F$${QMAKE_LIBDIR_QT}
		 FRAMEWORK_INCLUDE = $$QMAKE_LIBDIR_QT/$${QT_SHARED_LIB_NAME}.framework/Headers
		 exists($$FRAMEWORK_INCLUDE) {
		      INCLUDEPATH -= $$FRAMEWORK_INCLUDE
		      INCLUDEPATH = $$FRAMEWORK_INCLUDE $$INCLUDEPATH
		 }
		 QT_SHARED_LINKAGE = -framework $${QT_SHARED_LIB_NAME}
            } else:!qt_no_framework { #detection
	         for(frmwrk_dir, $$list($$QMAKE_LIBDIR_QT $$QMAKE_LIBDIR $$(DYLD_FRAMEWORK_PATH) /Library/Frameworks)) {
		      exists($${frmwrk_dir}/$${QT_SHARED_LIB_NAME}.framework) {
		         QMAKE_CFLAGS *= -F$${frmwrk_dir}
			 QMAKE_CXXFLAGS *= -F$${frmwrk_dir}
			 QMAKE_LIBDIR_FLAGS *= -F$${frmwrk_dir}
			 FRAMEWORK_INCLUDE = $$frmwrk_dir/$${QT_SHARED_LIB_NAME}.framework/Headers
			 INCLUDEPATH -= $$FRAMEWORK_INCLUDE
			 INCLUDEPATH = $$FRAMEWORK_INCLUDE $$INCLUDEPATH
		      }
		      QT_SHARED_LINKAGE = -framework $${QT_SHARED_LIB_NAME}
		      break()
	          }
            }
       }

       false {
           QT_SHARED_LINKAGE = -l$${QT_SHARED_LIB_NAME}
       } else:isEmpty(QT_SHARED_LINKAGE) {
          win32 {
             CONFIG(debug, debug|release):QT_SHARED_LINKAGE = -l$${QT_SHARED_LIB_NAME}d
             else:QT_SHARED_LINKAGE = -l$${QT_SHARED_LIB_NAME}
          } else { 
            isEqual(QT_SHARED_LIB_NAME_style, debug):QT_SHARED_LINKAGE = -l$${QT_SHARED_LIB_NAME}_debug
            else:QT_SHARED_LINKAGE = -l$${QT_SHARED_LIB_NAME}
          }
       }
    }
    LIBS += $$QT_SHARED_LINKAGE
}

