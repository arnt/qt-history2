TEMPLATE    = subdirs
SUBDIRS     = calculatorform

!static:SUBDIRS += calculatorbuilder \
                   containerextension \
                   customwidgetplugin \
                   taskmenuextension \
                   worldtimeclockbuilder \
                   worldtimeclockplugin

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer
INSTALLS += sources
