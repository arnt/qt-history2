TEMPLATE      = subdirs
unset(EXAMPLES_OPENGL_SUBDIRS)
EXAMPLES_OPENGL_SUBDIRS = examples_opengl_2dpainting \
                          examples_opengl_grabber \
                          examples_opengl_hellogl \
                          examples_opengl_overpainting \
                          examples_opengl_pbuffers \
                          examples_opengl_pbuffers2 \
                          examples_opengl_framebufferobject \
                          examples_opengl_framebufferobject2 \
                          examples_opengl_samplebuffers \
                          examples_opengl_textures


# install
target.path = $$[QT_INSTALL_EXAMPLES]/opengl
EXAMPLES_OPENGL_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS opengl.pro README
EXAMPLES_OPENGL_install_sources.path = $$[QT_INSTALL_EXAMPLES]/opengl
INSTALLS += target EXAMPLES_OPENGL_install_sources

#subdirs
examples_opengl_2dpainting.subdir = $$QT_BUILD_TREE/examples/opengl/2dpainting
examples_opengl_2dpainting.depends =  src_corelib src_gui src_opengl
examples_opengl_grabber.subdir = $$QT_BUILD_TREE/examples/opengl/grabber
examples_opengl_grabber.depends =  src_corelib src_gui src_opengl
examples_opengl_hellogl.subdir = $$QT_BUILD_TREE/examples/opengl/hellogl
examples_opengl_hellogl.depends =  src_corelib src_gui src_opengl
examples_opengl_overpainting.subdir = $$QT_BUILD_TREE/examples/opengl/overpainting
examples_opengl_overpainting.depends =  src_corelib src_gui src_opengl
examples_opengl_pbuffers.subdir = $$QT_BUILD_TREE/examples/opengl/pbuffers
examples_opengl_pbuffers.depends =  src_corelib src_gui src_opengl
examples_opengl_pbuffers2.subdir = $$QT_BUILD_TREE/examples/opengl/pbuffers2
examples_opengl_pbuffers2.depends =  src_corelib src_gui src_opengl src_svg
examples_opengl_framebufferobject.subdir = $$QT_BUILD_TREE/examples/opengl/framebufferobject
examples_opengl_framebufferobject.depends =  src_corelib src_gui src_opengl src_svg
examples_opengl_framebufferobject2.subdir = $$QT_BUILD_TREE/examples/opengl/framebufferobject2
examples_opengl_framebufferobject2.depends =  src_corelib src_gui src_opengl
examples_opengl_samplebuffers.subdir = $$QT_BUILD_TREE/examples/opengl/samplebuffers
examples_opengl_samplebuffers.depends =  src_corelib src_gui src_opengl
examples_opengl_textures.subdir = $$QT_BUILD_TREE/examples/opengl/textures
examples_opengl_textures.depends =  src_corelib src_gui src_opengl
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_OPENGL_SUBDIRS
SUBDIRS += $$EXAMPLES_OPENGL_SUBDIRS
