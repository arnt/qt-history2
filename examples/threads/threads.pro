TEMPLATE      = subdirs
SUBDIRS       = mandelbrot \
                semaphores \
                waitconditions

# install
target.path = $$[QT_INSTALL_EXAMPLES]/threads
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS threads.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/threads
INSTALLS += target sources
DEFINES += QT_USE_USING_NAMESPACE
