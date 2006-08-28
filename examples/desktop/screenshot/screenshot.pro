HEADERS             = screenshot.h
SOURCES             = main.cpp \
                      screenshot.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/screenshot
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS screenshot.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/widgets/screenshot
INSTALLS += target sources
