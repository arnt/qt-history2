TEMPLATE 	= app
TARGET		= designer

CONFIG 		-= moc
DESTDIR		= $$QT_BUILD_TREE/bin

SOURCES		+= main.cpp
INCLUDEPATH	+= ../designer
LIBS	+= -ldesignercore -lqui -lqassistantclient -L$$QT_BUILD_TREE/lib
win32 {
   RC_FILE	= designer.rc
   win32-g++ {
	TARGETDEPS += $$QT_BUILD_TREE/lib/libdesignercore.a
   } else {
	TARGETDEPS += $$QT_BUILD_TREE/lib/designercore.lib
   }
}
mac {
   RC_FILE	= designer.icns
   staticlib:CONFIG -= global_init_link_order #yuck
   staticlib:TARGETDEPS += $$QT_BUILD_TREE/lib/libdesignercore.a
}


target.path=$$bins.path
INSTALLS        += target
