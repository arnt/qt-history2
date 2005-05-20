SOURCES += semaphores.cpp
QT = core

# install
target.path = $$[QT_INSTALL_DATA]/examples/threads/semaphores
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS semaphores.pro
sources.path = $$[QT_INSTALL_DATA]/examples/threads/semaphores
INSTALLS += target sources
