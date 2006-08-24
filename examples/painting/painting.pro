TEMPLATE      = subdirs
unset(EXAMPLES_PAINTING_SUBDIRS)
EXAMPLES_PAINTING_SUBDIRS = examples_painting_basicdrawing \
                            examples_painting_concentriccircles \
                            examples_painting_fontsampler \
                            examples_painting_imagecomposition \
                            examples_painting_painterpaths \
                            examples_painting_svgviewer \
                            examples_painting_transformations

# install
target.path = $$[QT_INSTALL_EXAMPLES]/painting
EXAMPLES_PAINTING_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS painting.pro README
EXAMPLES_PAINTING_install_sources.path = $$[QT_INSTALL_EXAMPLES]/painting
INSTALLS += target EXAMPLES_PAINTING_install_sources

#subdirs
examples_painting_basicdrawing.subdir = $$QT_BUILD_TREE/examples/painting/basicdrawing
examples_painting_basicdrawing.depends =  src_corelib src_gui
examples_painting_concentriccircles.subdir = $$QT_BUILD_TREE/examples/painting/concentriccircles
examples_painting_concentriccircles.depends =  src_corelib src_gui
examples_painting_imagecomposition.subdir = $$QT_BUILD_TREE/examples/painting/imagecomposition
examples_painting_imagecomposition.depends =  src_corelib src_gui
examples_painting_painterpaths.subdir = $$QT_BUILD_TREE/examples/painting/painterpaths
examples_painting_painterpaths.depends =  src_corelib src_gui
examples_painting_svgviewer.subdir = $$QT_BUILD_TREE/examples/painting/svgviewer
examples_painting_svgviewer.depends =  src_corelib src_gui src_svg
examples_painting_transformations.subdir = $$QT_BUILD_TREE/examples/painting/transformations
examples_painting_transformations.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_PAINTING_SUBDIRS
SUBDIRS += $$EXAMPLES_PAINTING_SUBDIRS
