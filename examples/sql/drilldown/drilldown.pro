HEADERS = database.h \
          imageitem.h \
          informationwindow.h \
          view.h 
SOURCES = imageitem.cpp \
          informationwindow.cpp \
          main.cpp \
          view.cpp
QT += sql

# install
target.path = $$[QT_INSTALL_EXAMPLES]/sql/drilldown
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS drilldown.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sql/drilldown
INSTALLS += target sources
