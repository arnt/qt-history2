# Qt compat module

compat {
	COMPAT_P	= compat

	HEADERS += \
		sql/qsqlfieldinfo.h \
		sql/qsqlrecordinfo.h \
		sql/qdatatable.h \
		sql/qdataview.h \
                sql/qsqlcursor.h \
                sql/qsqlselectcursor.h \
                sql/qsqlform.h \
                sql/qsqlmanager_p.h \
                sql/qeditorfactory.h \
                sql/qsqleditorfactory.h \
                sql/qsqlpropertymap.h \
		sql/qdatabrowser.h

	SOURCES += \
		sql/qdatatable.cpp \
		sql/qdataview.cpp \
                sql/qsqlcursor.cpp \
                sql/qsqlselectcursor.cpp \
                sql/qsqlform.cpp \
                sql/qsqlmanager_p.cpp \
                sql/qeditorfactory.cpp \
                sql/qsqleditorfactory.cpp \
                sql/qsqlpropertymap.cpp \
		sql/qdatabrowser.cpp 
}
