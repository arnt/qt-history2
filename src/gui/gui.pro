TARGET		= QtGUI
DEFINES += QT_BUILD_GUI_LIB

include(../qbase.pri)

QT = core 
!win32:!embedded:!mac:CONFIG	   += x11
contains(QT_CONFIG, x11sm):CONFIG += x11sm
contains(QT_CONFIG, opengl):CONFIG += opengl

!contains(QT_CONFIG, cups):DEFINES += QT_NO_CUPS
!contains(QT_CONFIG, nis):DEFINES += QT_NO_NIS
contains(QT_CONFIG, tablet):DEFINES += QT_TABLET_SUPPORT

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
INCLUDEPATH *= $$QMAKE_INCDIR_QT/Qt3Compat
include(compat.pri)

QMAKE_LIBS += $$QMAKE_LIBS_GUI
