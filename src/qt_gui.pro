TARGET		= qt
include(qbase.pri)

QCONFIG = kernel 

!cups:DEFINES += QT_NO_CUPS
!nis:DEFINES += QT_NO_NIS
DEFINES += QT_GUI_LIB

#platforms
include($$QT_SOURCE_TREE/arch/$$ARCH/arch.pri)
x11:include($$KERNEL_CPP/qt_x11.pri)
mac:include($$KERNEL_CPP/qt_mac.pri)
embedded:include($$KERNEL_CPP/qt_qws.pri)

#modules
include($$KERNEL_CPP/qt_gui.pri)
include($$CANVAS_CPP/qt_canvas.pri)
include($$TABLE_CPP/qt_table.pri)
include($$WIDGETS_CPP/qt_widgets.pri)
include($$DIALOGS_CPP/qt_dialogs.pri)
include($$ICONVIEW_CPP/qt_iconview.pri)
include($$WORKSPACE_CPP/qt_workspace.pri)
include($$STYLES_CPP/qt_styles.pri)
include($$ACCESSIBLE_CPP/qt_accessible.pri)
embedded:include($$EMBEDDED_CPP/qt_embedded.pri)
qt_one_lib { #for compat
   #qnetwork
   include($$NETWORK_CPP/qt_network.pri) 

   #xml
   include($$XML_CPP/qt_xml.pri)

   #qopengl
   include($$OPENGL_CPP/qt_opengl.pri)

   #qsql
   include($$SQL_CPP/qt_sql.pri)

   #qtkernel
   include($$KERNEL_CPP/qt_kernel.pri)
   include($$THREAD_CPP/qt_thread.pri)
   include($$TOOLS_CPP/qt_tools.pri)
   include($$CODECS_CPP/qt_codecs.pri)
   include($$KERNEL_CPP/qt_gfx.pri)

   #qcompat
   message("Move compat/* files into libqt3compat")
   include($$COMPAT_CPP/qt_compat.pri)
}

