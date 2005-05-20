HEADERS       = regexpdialog.h
SOURCES       = regexpdialog.cpp \
                main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/tools/regexp
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS regexp.pro
sources.path = $$[QT_INSTALL_DATA]/examples/tools/regexp
INSTALLS += target sources
