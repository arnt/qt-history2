SOURCES += semaphores.cpp
QT = core

# install
target.path = $$[QT_INSTALL_EXAMPLES]/threads/semaphores
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS semaphores.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/threads/semaphores
INSTALLS += target sources
