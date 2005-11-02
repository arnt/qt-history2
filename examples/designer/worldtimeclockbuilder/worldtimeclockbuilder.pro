CONFIG      += form
SOURCES     = main.cpp
RESOURCES   = worldtimeclockbuilder.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/designer/worldtimeclockbuilder
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.ui *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/worldtimeclockbuilder
INSTALLS += target sources
