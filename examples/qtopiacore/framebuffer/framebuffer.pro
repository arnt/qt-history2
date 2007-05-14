TEMPLATE = app
TARGET = framebuffer

SOURCES += main.c

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtopiacore/framebuffer
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS framebuffer.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtopiacore/framebuffer
INSTALLS += target sources
