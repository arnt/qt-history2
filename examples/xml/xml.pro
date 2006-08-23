TEMPLATE      = subdirs
unset(EXAMPLES_XML_SUBDIRS)
EXAMPLES_XML_SUBDIRS = examples_xml_dombookmarks \
                       examples_xml_saxbookmarks

# install
target.path = $$[QT_INSTALL_EXAMPLES]/xml
EXAMPLES_XML_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS xml.pro README
EXAMPLES_XML_install_sources.path = $$[QT_INSTALL_EXAMPLES]/xml
INSTALLS += target EXAMPLES_XML_install_sources

#subdirs
examples_xml_dombookmarks.subdir = $$QT_BUILD_TREE/examples/xml/dombookmarks
examples_xml_dombookmarks.depends =  src_corelib src_gui src_xml
examples_xml_saxbookmarks.subdir = $$QT_BUILD_TREE/examples/xml/saxbookmarks
examples_xml_saxbookmarks.depends =  src_corelib src_gui src_xml
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_XML_SUBDIRS
SUBDIRS += $$EXAMPLES_XML_SUBDIRS
