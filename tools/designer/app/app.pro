TEMPLATE 	= app
DESTDIR		= $$QT_BUILD_TREE/bin
TARGET		= designer
CONFIG 		-= moc

SOURCES		+= main.cpp
INCLUDEPATH	+= ../designer
unix:LIBS	+= -ldesigner -L$$QT_BUILD_TREE/lib
win32 {
   RC_FILE	= designer.rc
   LIBS	+= $$QT_BUILD_TREE/lib/designerlib.lib
}
mac {
   RC_FILE	= designer.icns
   LIBS	+= -lqui -lqassistantclient
   staticlib:CONFIG -= global_init_link_order #yuck
}


target.path=$$bins.path
INSTALLS        += target
