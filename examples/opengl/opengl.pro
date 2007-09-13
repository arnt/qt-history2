TEMPLATE      = subdirs
SUBDIRS       = 2dpainting \
                grabber \
                hellogl \
                overpainting \
                pbuffers \
                pbuffers2 \
		framebufferobject \
		framebufferobject2 \
                samplebuffers \
                textures


# install
target.path = $$[QT_INSTALL_EXAMPLES]/opengl
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS opengl.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/opengl
INSTALLS += target sources
DEFINES += QT_USE_USING_NAMESPACE
