TARGET		= QtGui
QPRO_PWD        = $$PWD
DEFINES += QT_BUILD_GUI_LIB

include(../qbase.pri)

QT = core 
!win32:!embedded:!mac:CONFIG	  += x11
contains(QT_CONFIG, x11sm):CONFIG += x11sm

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

embedded: INCLUDEPATH *= $$QMAKE_INCDIR_QT/QtNetwork

QMAKE_LIBS += $$QMAKE_LIBS_GUI
