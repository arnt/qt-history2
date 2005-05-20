HEADERS       = analogclock.h
SOURCES       = analogclock.cpp \
                main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/widgets/analogclock
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS analogclock.pro
sources.path = $$[QT_INSTALL_DATA]/examples/widgets/analogclock
INSTALLS += target sources
