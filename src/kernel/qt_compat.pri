# Qt compatibility

# scratch pad for internal development
# hack these for your build like
# internal {
#     CONFIG += blah
# }

##########################################################

# mac hac fu
mac {
     #we always use these
     CONFIG += zlib png
     #never
     CONFIG -= nas mng x11 x11sm
     
     #CONFIG += shared
     #CONFIG += sqlcrap
     sql:sqlcrap {
	sql-drivers += postgres
	INCLUDEPATH+=/Users/sam/postgresql-7.0.2/src/include \
		    /Users/sam/postgresql-7.0.2/src/interfaces/libpq
	LIBS += -L/Users/sam/postgresql-7.0.2/src/interfaces/libpq 
     }
} 

