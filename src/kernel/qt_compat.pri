# Qt compatibility

# scratch pad for internal development

# hack these for your build

# basic internal setup
internal {
	MODULES_BASE	= tools kernel widgets dialogs styles
	MODULES_PRO	= iconview workspace
	MODULES_ENT	= network canvas table xml opengl sql
	# we want everything
	CONFIG		+= $$MODULES_BASE $$MODULES_PRO $$MODULES_ENT
	CONFIG	+= png zlib  # Done differently in external system
	CONFIG  += x11sm
#	CONFIG += nas
#	CONFIG -= opengl
	# Install jpegsrc.v6b.tar.gz (find with http://ftpsearch.lycos.com)
	unix:CONFIG += jpeg
#	LIBS += -lpng -lz
#	CONFIG += mng
}

##########################################################

# mac hac fu
macx-g++ {
     #we always use these
     CONFIG += tools kernel widgets dialogs iconview workspace \
               network canvas table xml zlib png styles sql
     #never
     CONFIG -= nas mng jpeg x11 x11sm
     
     CONFIG += shared debug
#     CONFIG += sqlcrap
     sqlcrap {
	sql-driver += postgres
	INCLUDEPATH+=/Users/sam/postgresql-7.0.2/src/include \
		    /Users/sam/postgresql-7.0.2/src/interfaces/libpq
	LIBS += -L/Users/sam/postgresql-7.0.2/src/interfaces/libpq 
     }
} 

