TEMPLATE      = subdirs
SUBDIRS       = textfinder \
                multipleinheritance

# install
target.path = $$[QT_INSTALL_EXAMPLES]/uitools
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS uitools.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/uitools
INSTALLS += target sources
