TARGET		= qt
DEFINES += QT_BUILD_GUI_LIB

include(../qbase.pri)

QT = core 
!win32:!embedded:!mac:CONFIG	   += x11 x11inc

!cups:DEFINES += QT_NO_CUPS
!nis:DEFINES += QT_NO_NIS


#platforms
x11:include(kernel/x11.pri)
mac:include(kernel/mac.pri)
win32:include(kernel/win.pri)
embedded:include(embedded/embedded.pri)

#modules
include(kernel/kernel.pri)
include(image/image.pri)
include(painting/painting.pri)
include(text/text.pri)
include(styles/styles.pri)
include(widgets/widgets.pri)
include(dialogs/dialogs.pri)
include(accessible/accessible.pri)
include(itemviews/itemviews.pri)

# ##### this should go away eventually
include(compat.pri)

QMAKE_LIBS += $$QMAKE_LIBS_GUI
