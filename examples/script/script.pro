TEMPLATE = subdirs
SUBDIRS = helloscript qscript context2d marshal defaultprototypes

!cross_compile:SUBDIRS += calculator tetrix

# install
target.path = $$[QT_INSTALL_EXAMPLES]/script
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS script.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/script
INSTALLS += target sources
