mac {
	DEFINES += CARBON_ON_MACH_O=1 ALLOW_OLD_BLOCKING_APIS=0 ALLOW_OLD_EVENT_LOOP_APIS=0
	INCLUDEPATH += /System/Library/Frameworks/QuickTime.framework/Headers/ \
	               /System/Library/Frameworks/Carbon.framework/Headers/ \
		       /System/Library/Frameworks/HIToolbox.framework/Headers \
		       /System/Library/Frameworks/PrintCore.framework/Headers \
 /System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreGraphics.framework/Headers/
	LIBS += -framework Carbon -framework QuickTime
}
