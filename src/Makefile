#############################################################################
# Qt Global Makefile
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib
DISTDIR		= /tmp/qt-0.96

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -W -Wtemplate-debugging -Wparentheses -Wuninitialized \
		-Wchar-subscripts -Wformat -Wtrigraphs -Wcomment -Wswitch \
		-Wunused -Wreturn-type -Wimplicit -Wpointer-arith -Wsynth \
		-Wconversion -Wno-overloaded-virtual \
		-O2 -fno-strength-reduce
CDBGFLAGS	= -g -W -Wall -Wpointer-arith -Wsynth -Wconversion
CSTATICFLAGS	=
CSHAREDFLAGS	= -fPIC
CC		= gcc

####### Targets

STATICTARGET	= $(LIBDIR)/libqt.a
SHAREDTARGET	= $(LIBDIR)/libqt.so.0.9.6

#######	Sub directories (excluding moc)

DIRS		= tools kernel widgets dialogs doc

#######	Build rules

all:
	@echo "PLEASE SPECIFY A TARGET"
	@echo "  make static    - creates a static library"
	@echo "  make staticdbg - creates a static library w/debug info"
	@echo "  make shared    - creates a shared library"
	@echo "  make shareddbg - creates a shared library w/debug info"
	@echo "  make moc       - creates the meta object compiler"
	@echo "  make tools     - creates the tools library"
	@echo "  make dist      - make libraries for a distribution"
	@exit 1

#
# Create the meta object compiler
#

moc: tools mochack

mochack:
	$(MAKE) -C moc

#
# Create a library, called from static, staticdbg, shared and shareddbg
#

FORCE:


lib: tools kernel widgets dialogs

# need FORCE because the directories' dates confuse make terribly
tools kernel widgets dialogs: FORCE
	-mkdir -p $(OBJDIR)/$@
	-rm -f $@/*.o
	-ln -f $(OBJDIR)/$@/*.o $@
	$(MAKE) -C $@ obj
	rm -f $(OBJDIR)/$@/*.o
	mv -f $@/*.o $(OBJDIR)/$@

static:
	$(MAKE) lib 'CFLAGS=$(CFLAGS) $(CSTATICFLAGS)' 'OBJDIR=obj/static'
	/bin/rm -f $(STATICTARGET)
	ar q $(STATICTARGET) obj/static/*/*.o
	ranlib $(STATICTARGET)

staticdbg:
	$(MAKE) lib 'CFLAGS=$(CDBGFLAGS) $(CSTATICFLAGS)' 'OBJDIR=obj/staticdbg'
	/bin/rm -f $(STATICTARGET)
	ar q $(STATICTARGET) obj/staticdbg/*/*.o
	ranlib $(STATICTARGET)

shared:
	$(MAKE) lib 'CFLAGS=$(CFLAGS) $(CSHAREDFLAGS)' 'OBJDIR=obj/shared'
	/bin/rm -f $(SHAREDTARGET)
	$(CC) -shared -Wl,-soname,libqt.so.0 -o $(SHAREDTARGET) \
		obj/shared/*/*.o -lX11 -lgcc -lc

shareddbg:
	$(MAKE) lib 'CFLAGS=$(CDBGFLAGS) $(CSHAREDFLAGS)' 'OBJDIR=obj/shareddbg'
	/bin/rm -f $(SHAREDTARGET)
	$(CC) -shared -o -Wl,-soname,libqt.so.0 -o $(SHAREDTARGET) \
		obj/shareddbg/*/*.o -lX11 -lgcc -lc

dist: 
	-$(RM) -rf $(DISTDIR)
	mkdir -p $(DISTDIR)/{include,lib,bin,man/man3,man/man1,html,src,arch}
	(tar cf - . )|(cd $(DISTDIR)/src ; tar xf -)
	mkdir -p `make -s -C /local/qt/examples showdirs | fmt -1 | sed 's_^_'$(DISTDIR)/examples/_g`
	set -e ; for a in `find $(DISTDIR)/examples -type d -print` ; \
	do \
	    ( set -e ; cd $$a ; co -q -r `pwd | sed s@$(DISTDIR)/@/local/qt/@`/RCS/*,v ); \
	done
	mkdir -p `make -s -C /local/qt/tutorial showdirs | fmt -1 | sed 's_^_'$(DISTDIR)/tutorial/_g`
	set -e ; for a in `find $(DISTDIR)/tutorial -type d -print` ; \
	do \
	    ( set -e ; cd $$a ; co -q -r `pwd | sed s@$(DISTDIR)/@/local/qt/@`/RCS/*,v ); \
	done
	( cd $(DISTDIR)/src ; $(QTDIR)/util/makearch.sh )
	-rm -rf $(DISTDIR)/src/{obj,doc,gif,qdoc,moc/test,makefiles}
	find $(DISTDIR)/src \( -name \*.o -o -name \*_win.\* -o -name \*_os2.\* -o -name Makefile -o -name RCS \) -print | xargs rm -v
	co -q -p -q moc/moc.1 > $(DISTDIR)/man/man1/moc.1
	cp -f /local/man/man3/*.3qt $(DISTDIR)/man/man3
	cp -f /local/qt/doc/html/*.html $(DISTDIR)/html
	cp -f /local/qt/doc/html/*.gif $(DISTDIR)/html
	chmod -R u+w $(DISTDIR)

rpm:
	rpm -bi --short-circuit qt-0.96-1.spec
	rpm -bl qt-0.96-1.spec
	rpm -bb qt-0.96-1.spec

tar:
	-rm $(DISTDIR).tar.gz
	( set -e ; cd $(DISTDIR) ; tar cf misc.tar bin lib man )
	( set -e ; cd $(LIBDIR) ; tar cf $(DISTDIR)/shared.tar libqt.so* )
	( set -e ; cd $(LIBDIR) ; tar cf $(DISTDIR)/static.tar libqt.a )
	( set -e ; cd $(DISTDIR)/.. ; tar cf $(DISTDIR).tar `basename $(DISTDIR)`/{README,INSTALL,misc.tar,static.tar,shared.tar,CHANGES,ANNOUNCE,MANIFEST,LICENCE} ; gzip -f9 $(DISTDIR).tar )

info:
	@for d in $(DIRS); do cd $$d ; rinfo ; cd .. ; done

newer:
	@for d in $(DIRS); do cd $$d ; rnewer ; cd .. ; done

update:
	@for d in $(DIRS); do cd $$d ; rupdate ; cd .. ; done

checkout: 
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d checkout ; done

showdirs:
	@echo $(DIRS)

showfiles:
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d showfiles ; done

showsources:
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d showsources ; done

showheaders:
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d showheaders ; done

clean:
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d clean ; done

depend dep:
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d depend ; done

include:
	-rm $(INCDIR)/*.h
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d include ; done

tags:
	echo */*.cpp */*.h | fmt -1 | egrep -v '_win|_os2|moc/' | etags --c++ -
