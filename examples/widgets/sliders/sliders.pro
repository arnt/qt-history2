HEADERS     = slidersgroup.h \
              window.h
SOURCES     = main.cpp \
              slidersgroup.cpp \
              window.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/widgets/sliders
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS sliders.pro
sources.path = $$[QT_INSTALL_DATA]/examples/widgets/sliders
INSTALLS += target sources
