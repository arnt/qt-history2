TEMPLATE      = subdirs
SUBDIRS       = mandelbrot \
                semaphores \
                waitconditions

# install
target.path = $$[QT_INSTALL_DATA]/examples/threads
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS threads.pro README
sources.path = $$[QT_INSTALL_DATA]/examples/threads
INSTALLS += target sources
