# Qt styles module

styles {
	HEADERS +=$$STYLES_H/qstylefactory.h \
		  $$STYLES_H/qstyleinterface.h \
		  $$STYLES_H/qcommonstyle.h \
		  $$STYLES_H/qwindowsstyle.h \
		  $$STYLES_H/qplatinumstyle.h \
		  $$STYLES_H/qmotifstyle.h \
		  $$STYLES_H/qcdestyle.h
#		  $$STYLES_H/qmotifplusstyle.h \
#		  $$STYLES_H/qinterlacestyle.h \
#		  $$STYLES_H/qsgistyle.h \
#		  $$STYLES_H/qcompactstyle.h

	SOURCES +=$$STYLES_CPP/qstylefactory.cpp \
		  $$STYLES_CPP/qcommonstyle.cpp \
		  $$STYLES_CPP/qwindowsstyle.cpp \
		  $$STYLES_CPP/qplatinumstyle.cpp \
		  $$STYLES_CPP/qmotifstyle.cpp \
		  $$STYLES_CPP/qcdestyle.cpp
#		  $$STYLES_CPP/qmotifplusstyle.cpp \
#		  $$STYLES_CPP/qinterlacestyle.cpp \
#		  $$STYLES_CPP/qsgistyle.cpp \
#		  $$STYLES_CPP/qcompactstyle.cpp

}

!mac:DEFINES += QT_NO_STYLE_AQUA
mac {
	HEADERS +=$$STYLES_H/qaquastyle.h 
	SOURCES +=$$STYLES_CPP/qaquastyle.cpp 
#	HEADERS +=$$STYLES_H/qmacstyle_mac.h
#	SOURCES +=$$STYLES_CPP/qmacstyle_mac.cpp
}
