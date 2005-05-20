HEADERS       = digitalclock.h
SOURCES       = digitalclock.cpp \
                main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/widgets/digitalclock
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS digitalclock.pro
sources.path = $$[QT_INSTALL_DATA]/examples/widgets/digitalclock
INSTALLS += target sources
