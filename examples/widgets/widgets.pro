TEMPLATE      = subdirs
unset(EXAMPLES_WIDGETS_SUBDIRS)
EXAMPLES_WIDGETS_SUBDIRS = examples_widgets_analogclock \
                           examples_widgets_calculator \
                           examples_widgets_charactermap \
                           examples_widgets_digitalclock \
                           examples_widgets_groupbox \
                           examples_widgets_icons \
                           examples_widgets_imageviewer \
                           examples_widgets_lineedits \
                           examples_widgets_movie \
                           examples_widgets_scribble \
                           examples_widgets_shapedclock \
                           examples_widgets_sliders \
                           examples_widgets_spinboxes \
                           examples_widgets_styles \
                           examples_widgets_tetrix \
                           examples_widgets_tooltips \
                           examples_widgets_wiggly \
                           examples_widgets_windowflags

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets
EXAMPLES_WIDGETS_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS widgets.pro README
EXAMPLES_WIDGETS_install_sources.path = $$[QT_INSTALL_EXAMPLES]/widgets
INSTALLS += target EXAMPLES_WIDGETS_install_sources

#subdirs
examples_widgets_analogclock.subdir = $$QT_BUILD_TREE/examples/widgets/analogclock
examples_widgets_analogclock.depends =  src_corelib src_gui
examples_widgets_calculator.subdir = $$QT_BUILD_TREE/examples/widgets/calculator
examples_widgets_calculator.depends =  src_corelib src_gui
examples_widgets_charactermap.subdir = $$QT_BUILD_TREE/examples/widgets/charactermap
examples_widgets_charactermap.depends =  src_corelib src_gui
examples_widgets_digitalclock.subdir = $$QT_BUILD_TREE/examples/widgets/digitalclock
examples_widgets_digitalclock.depends =  src_corelib src_gui
examples_widgets_groupbox.subdir = $$QT_BUILD_TREE/examples/widgets/groupbox
examples_widgets_groupbox.depends =  src_corelib src_gui
examples_widgets_icons.subdir = $$QT_BUILD_TREE/examples/widgets/icons
examples_widgets_icons.depends =  src_corelib src_gui
examples_widgets_imageviewer.subdir = $$QT_BUILD_TREE/examples/widgets/imageviewer
examples_widgets_imageviewer.depends =  src_corelib src_gui
examples_widgets_lineedits.subdir = $$QT_BUILD_TREE/examples/widgets/lineedits
examples_widgets_lineedits.depends =  src_corelib src_gui
examples_widgets_movie.subdir = $$QT_BUILD_TREE/examples/widgets/movie
examples_widgets_movie.depends =  src_corelib src_gui
examples_widgets_scribble.subdir = $$QT_BUILD_TREE/examples/widgets/scribble
examples_widgets_scribble.depends =  src_corelib src_gui
examples_widgets_shapedclock.subdir = $$QT_BUILD_TREE/examples/widgets/shapedclock
examples_widgets_shapedclock.depends =  src_corelib src_gui
examples_widgets_sliders.subdir = $$QT_BUILD_TREE/examples/widgets/sliders
examples_widgets_sliders.depends =  src_corelib src_gui
examples_widgets_spinboxes.subdir = $$QT_BUILD_TREE/examples/widgets/spinboxes
examples_widgets_spinboxes.depends =  src_corelib src_gui
examples_widgets_styles.subdir = $$QT_BUILD_TREE/examples/widgets/styles
examples_widgets_styles.depends =  src_corelib src_gui
examples_widgets_tetrix.subdir = $$QT_BUILD_TREE/examples/widgets/tetrix
examples_widgets_tetrix.depends =  src_corelib src_gui
examples_widgets_tooltips.subdir = $$QT_BUILD_TREE/examples/widgets/tooltips
examples_widgets_tooltips.depends =  src_corelib src_gui
examples_widgets_wiggly.subdir = $$QT_BUILD_TREE/examples/widgets/wiggly
examples_widgets_wiggly.depends =  src_corelib src_gui
examples_widgets_windowflags.subdir = $$QT_BUILD_TREE/examples/widgets/windowflags
examples_widgets_windowflags.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_WIDGETS_SUBDIRS
SUBDIRS += $$EXAMPLES_WIDGETS_SUBDIRS
