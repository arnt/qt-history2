TEMPLATE 	= app
DESTDIR		= $$QT_BUILD_TREE/bin
TARGET		= designer
CONFIG 		-= moc

SOURCES		+= main.cpp
INCLUDEPATH	+= ../designer
unix:LIBS	+= -ldesigner -lqui -lqassistantclient -L$$QT_BUILD_TREE/lib
win32 {
   RC_FILE	= designer.rc
   win32-g++ {
   	 LIBS 	+= $$QT_BUILD_TREE/lib/libdesignerlib.a $$QT_BUILD_TREE/lib/libqassistantclient.a $$QT_BUILD_TREE/lib/libqui.a
   } else {
       LIBS 	+= $$QT_BUILD_TREE/lib/designerlib.lib
       win32-msvc.net:LIBS 	+= $$QT_BUILD_TREE/lib/qassistantclient.lib $$QT_BUILD_TREE/lib/qui.lib
   }
}
mac {
   RC_FILE	= designer.icns
   LIBS	+= -lqui
   staticlib:CONFIG -= global_init_link_order #yuck
}


target.path=$$bins.path
INSTALLS        += target
