# Qt compatibility

# scratch pad for internal development

# hack these for your build
hacks = oldrichtext
# hacks += newrichtext
hacks += oldx11font
# hacks += newx11font

# basic internal setup
internal {
	MODULES_BASE	= tools kernel widgets dialogs
	MODULES_PRO	= iconview workspace
	MODULES_ENT	= network canvas table xml opengl sql
	# we want everything
	CONFIG		+= $$MODULES_BASE $$MODULES_PRO $$MODULES_ENT
	CONFIG	+= png zlib  # Done differently in external system
	CONFIG  += x11sm
#	CONFIG += nas
	CONFIG -= opengl
	# Install jpegsrc.v6b.tar.gz (find with http://ftpsearch.lycos.com)
	unix:CONFIG += jpeg
#	LIBS += -lpng -lz
#	CONFIG += mng
}

##########################################################

unix:!embedded {
	contains(hacks,oldx11font):SOURCES += kernel/qfont_x11.cpp
	contains(hacks,newx11font):SOURCES += ../tests/newfont/qfont_newx11.cpp
}


