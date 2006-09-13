HEADERS     = mainwindow.h
FORMS       = forms/stylesheeteditor.ui
RESOURCES   = stylesheet.qrc
SOURCES     = mainwindow.cpp \
              main.cpp
CONFIG      += uitools console

# install
target.path = $$[QT_INSTALL_EXAMPLES]/designer/stylesheet
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/stylesheet
INSTALLS += target sources
