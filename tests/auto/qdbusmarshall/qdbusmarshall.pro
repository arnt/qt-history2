contains(QT_CONFIG,qdbus): {
	load(qttest_p4)
	TEMPLATE = subdirs
	CONFIG += ordered
	SUBDIRS = qpong test
} else {
	SOURCES += dummy.cpp
}
