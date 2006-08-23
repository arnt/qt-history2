TEMPLATE    = subdirs
unset(EXAMPLES_DESIGNER_SUBDIRS)
EXAMPLES_DESIGNER_SUBDIRS = examples_designer_calculatorform

!static:EXAMPLES_DESIGNER_SUBDIRS += examples_designer_calculatorbuilder \
                                     examples_designer_containerextension \
                                     examples_designer_customwidgetplugin \
                                     examples_designer_taskmenuextension \
                                     examples_designer_worldtimeclockbuilder \
                                     examples_designer_worldtimeclockplugin

# the sun cc compiler has a problem with the include lines for the form.prf
solaris-cc*:EXAMPLES_DESIGNER_SUBDIRS -= examples_designer_calculatorbuilder \
                                         examples_designer_worldtimeclockbuilder
		     

# install
EXAMPLES_DESIGNER_install_sources.files = README *.pro
EXAMPLES_DESIGNER_install_sources.path = $$[QT_INSTALL_EXAMPLES]/designer
INSTALLS += EXAMPLES_DESIGNER_install_sources

#subdirs
examples_designer_calculatorform.subdir = $$QT_BUILD_TREE/examples/designer/calculatorform
examples_designer_calculatorform.depends =  src_corelib src_gui
examples_designer_calculatorbuilder.subdir = $$QT_BUILD_TREE/examples/designer/calculatorbuilder
examples_designer_calculatorbuilder.depends =  src_corelib src_gui
examples_designer_containerextension.subdir = $$QT_BUILD_TREE/examples/designer/containerextension
examples_designer_containerextension.depends =  src_corelib src_gui tools_designer_src_lib
examples_designer_customwidgetplugin.subdir = $$QT_BUILD_TREE/examples/designer/customwidgetplugin
examples_designer_customwidgetplugin.depends =  src_corelib src_gui tools_designer_src_lib
examples_designer_taskmenuextension.subdir = $$QT_BUILD_TREE/examples/designer/taskmenuextension
examples_designer_taskmenuextension.depends =  src_corelib src_gui tools_designer_src_lib
examples_designer_worldtimeclockbuilder.subdir = $$QT_BUILD_TREE/examples/designer/worldtimeclockbuilder
examples_designer_worldtimeclockbuilder.depends =  src_corelib src_gui
examples_designer_worldtimeclockplugin.subdir = $$QT_BUILD_TREE/examples/designer/worldtimeclockplugin
examples_designer_worldtimeclockplugin.depends =  src_corelib src_gui tools_designer_src_lib
examples_designer_calculatorbuilder.subdir = $$QT_BUILD_TREE/examples/designer/calculatorbuilder
examples_designer_calculatorbuilder.depends =  src_corelib src_gui
examples_designer_worldtimeclockbuilder.subdir = $$QT_BUILD_TREE/examples/designer/worldtimeclockbuilder
examples_designer_worldtimeclockbuilder.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_DESIGNER_SUBDIRS
SUBDIRS += $$EXAMPLES_DESIGNER_SUBDIRS
