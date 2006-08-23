TEMPLATE = subdirs
CONFIG += 
CONFIG -= app_bundle
unset(TOOLS_QDBUS_TOOLS_SUBDIRS)
TOOLS_QDBUS_TOOLS_SUBDIRS = tools_qdbus_tools_dbus \
                            tools_qdbus_tools_dbusxml2cpp \
                            tools_qdbus_tools_dbuscpp2xml

#subdirs
tools_qdbus_tools_dbus.subdir = $$QT_BUILD_TREE/tools/qdbus/tools/dbus
tools_qdbus_tools_dbus.depends =  src_corelib
tools_qdbus_tools_dbusxml2cpp.subdir = $$QT_BUILD_TREE/tools/qdbus/tools/dbusxml2cpp
tools_qdbus_tools_dbusxml2cpp.depends =  src_corelib src_xml
tools_qdbus_tools_dbuscpp2xml.subdir = $$QT_BUILD_TREE/tools/qdbus/tools/dbuscpp2xml
tools_qdbus_tools_dbuscpp2xml.depends =  src_corelib src_xml
TOOLS_QDBUS_SUB_SUBDIRS += $$TOOLS_QDBUS_TOOLS_SUBDIRS
TOOLS_SUB_SUBDIRS += $$TOOLS_QDBUS_TOOLS_SUBDIRS
SUBDIRS += $$TOOLS_QDBUS_TOOLS_SUBDIRS
