TARGET  = qtscriptgui
include(../../qpluginbase.pri)
QT = core gui script

SOURCES += main.cpp \
	   brush.cpp \
	   color.cpp \
	   font.cpp \
	   gradient.cpp \
	   conicalgradient.cpp \
	   lineargradient.cpp \
	   radialgradient.cpp \
	   icon.cpp \
	   image.cpp \
	   keyevent.cpp \
	   matrix.cpp \
	   mouseevent.cpp \
	   pen.cpp \
	   pixmap.cpp \
	   polygon.cpp \
	   painter.cpp \
	   painterpath.cpp \
	   palette.cpp \
	   region.cpp \
	   transform.cpp

SOURCES += graphicsitem.cpp \
	   graphicsitemanimation.cpp \
	   graphicsitemgroup.cpp \
	   abstractgraphicsshapeitem.cpp \
	   graphicsellipseitem.cpp \
	   graphicslineitem.cpp \
	   graphicspathitem.cpp \
	   graphicspixmapitem.cpp \
	   graphicspolygonitem.cpp \
	   graphicsrectitem.cpp \
	   graphicssimpletextitem.cpp \
	   graphicstextitem.cpp \
	   graphicsscene.cpp \
	   graphicsview.cpp

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/script
target.path += $$[QT_INSTALL_PLUGINS]/script
INSTALLS += target
