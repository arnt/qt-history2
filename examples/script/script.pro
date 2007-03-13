TEMPLATE = subdirs
SUBDIRS = helloscript qscript context2d marshal calculator tetrix defaultprototypes

# install
target.path = $$[QT_INSTALL_EXAMPLES]/script
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS script.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/script
INSTALLS += target sources
