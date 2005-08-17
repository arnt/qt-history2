TEMPLATE    = subdirs
SUBDIRS     = calculatorform \
              worldtimeclockplugin
!static:SUBDIRS += calculatorbuilder \
                   customwidgetplugin \
                   taskmenuextension

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer
INSTALLS += sources
