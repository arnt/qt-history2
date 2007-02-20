HEADERS += rsslisting.h
SOURCES += main.cpp rsslisting.cpp
CONFIG += qt warn_on debug  create_prl link_prl
QT += network xml

# install
target.path = $$[QT_INSTALL_EXAMPLES]/xml/rsslisting
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS rsslisting.pro 
sources.path = $$[QT_INSTALL_EXAMPLES]/xml/rsslisting
INSTALLS += target sources

