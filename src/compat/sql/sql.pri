# Qt compat module

compat {
	COMPAT_P	= compat

	HEADERS += \
		sql/qsqlfieldinfo.h \
		sql/qsqlrecordinfo.h \
		sql/qdatatable.h \
		sql/qdataview.h \
		sql/qdatabrowser.h

	SOURCES += \
		sql/qdatatable.cpp \
		sql/qdataview.cpp \
		sql/qdatabrowser.cpp 
}
