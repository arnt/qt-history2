TARGET	 = qgfxsnap
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/gfxdrivers

DEFINES	+= QT_QWS_SNAP

HEADERS		= ../../../../include/Qt/qgfxsnap_qws.h
SOURCES		= main.cpp \
		  ../../../../src/gui/embedded/qgfxsnap_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS += target

INCLUDEPATH += $(SCITECH)/include

debug:LIBS  += -L$(SCITECH)/lib/debug/linux/gcc/x86/so -lpm
else:LIBS   += -L$(SCITECH)/lib/release/linux/gcc/x86/so -lpm
