HEADERS       = button.h \
                calculator.h
SOURCES       = button.cpp \
                calculator.cpp \
                main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/widgets/calculator
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS calculator.pro
sources.path = $$[QT_INSTALL_DATA]/examples/widgets/calculator
INSTALLS += target sources
