load(qttest_p4)
QT = core
contains(QT_CONFIG,qdbus): {
	SOURCES += tst_qdbusxmlparser.cpp
	CONFIG += qdbus
} else {
	SOURCES += ../qdbusmarshall/dummy.cpp
}
