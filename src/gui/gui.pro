TARGET		= qt
DEFINES += QT_BUILD_GUI_LIB

include(../qbase.pri)

QCONFIG = core 
!win32:!embedded:!mac:CONFIG	   += x11 x11inc

!cups:DEFINES += QT_NO_CUPS
!nis:DEFINES += QT_NO_NIS


#platforms
x11:include(base/x11.pri)
mac:include(base/mac.pri)
win32:include(base/win.pri)
embedded:include(embedded/embedded.pri)

#modules
include(base/base.pri)
include(image/image.pri)
include(painting/painting.pri)
include(text/text.pri)
include(styles/styles.pri)
include(widgets/widgets.pri)
include(dialogs/dialogs.pri)
include(accessible/accessible.pri)

# ##### this should go away eventually
include(compat.pri)
