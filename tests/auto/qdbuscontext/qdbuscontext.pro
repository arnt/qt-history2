load(qttest_p4)
QT = core

contains(QT_CONFIG,qdbus): {
	SOURCES += tst_qdbuscontext.cpp
	CONFIG += qdbus
} else {
	SOURCES += ../qdbusmarshall/dummy.cpp
}

DEFINES += QT_USE_USING_NAMESPACE

