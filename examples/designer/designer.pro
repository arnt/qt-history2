TEMPLATE    = subdirs
SUBDIRS     = calculatorbuilder \
              calculatorform \
              customwidgetplugin \
              worldtimeclockplugin

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/designer
INSTALLS += sources
