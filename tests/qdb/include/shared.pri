CONFIG		+= qt warn_on debug console
DEFINES		+= QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT
INCLUDEPATH	+= ../include
SOURCES	        += ../src/sqlinterpreter.cpp \
		../src/environment.cpp \
		../src/filedriver_xbase.cpp \
		../src/op.cpp \
		../src/parser.cpp \
		../src/qdb.cpp 
xbase {
	XBASE_ROOT	= ../xdb-1.2.0
	XBASE_PATH	= $$XBASE_ROOT/xdb
	DEFINES		+= HAVE_CONFIG_H
	SOURCES		+= $$XBASE_PATH/dbf.cpp \
			$$XBASE_PATH/fields.cpp\
			$$XBASE_PATH/index.cpp\
			$$XBASE_PATH/ndx.cpp\
			$$XBASE_PATH/xbase.cpp\
			$$XBASE_PATH/xbstring.cpp\
			$$XBASE_PATH/stack.cpp\
			$$XBASE_PATH/exp.cpp\
			$$XBASE_PATH/expproc.cpp\
			$$XBASE_PATH/expfunc.cpp\
			$$XBASE_PATH/xdate.cpp
	INCLUDEPATH	+= $$XBASE_PATH $$XBASE_ROOT
}

