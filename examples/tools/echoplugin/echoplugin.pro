TEMPLATE    = subdirs
SUBDIRS	    = echowindow \
	      plugin

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/echoplugin
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS echoplugin.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/echoplugin
INSTALLS += target sources
