#############################################################################
# Qt Global Makefile
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib
DISTDIR		= /tmp/qt-0.93

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CDBGFLAGS	= -g
CSTATICFLAGS	=
CSHAREDFLAGS	= -fPIC -b i486-linux
CC		= gcc

####### Targets

STATICTARGET	= $(LIBDIR)/libqt.a
SHAREDTARGET	= $(LIBDIR)/libqt.so.0.9.3

#######	Sub directories (excluding moc)

DIRS		= tools kernel widgets dialogs

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
	make -C moc

#
# Create a library, called from static, staticdbg, shared and shareddbg
#

FORCE:


lib: tools kernel widgets dialogs

tools kernel widgets dialogs: FORCE
	-mkdir -p $(OBJDIR)/$@
	-rm -f $@/*.o
	-ln -f $(OBJDIR)/$@/*.o $@
	make -C $@ obj
	rm -f $(OBJDIR)/$@/*.o
	mv -f $@/*.o $(OBJDIR)/$@

static:
	make lib 'CFLAGS=$(CFLAGS) $(CSTATICFLAGS)' 'OBJDIR=obj/static'
	/bin/rm -f $(STATICTARGET)
	ar q $(STATICTARGET) obj/static/*/*.o
	ranlib $(STATICTARGET)

staticdbg:
	make lib 'CFLAGS=$(CDBGFLAGS) $(CSTATICFLAGS)' 'OBJDIR=obj/staticdbg'
	/bin/rm -f $(STATICTARGET)
	ar q $(STATICTARGET) obj/staticdbg/*/*.o
	ranlib $(STATICTARGET)

shared:
	make lib 'CFLAGS=$(CFLAGS) $(CSHAREDFLAGS)' 'OBJDIR=obj/shared'
	/bin/rm -f $(SHAREDTARGET)
	$(CC) -shared -o $(SHAREDTARGET) obj/shared/*/*.o -lX11 -lgcc -lc

shareddbg:
	make lib 'CFLAGS=$(CDBGFLAGS) $(CSHAREDFLAGS)' 'OBJDIR=obj/shareddbg'
	/bin/rm -f $(SHAREDTARGET)
	$(CC) -shared -o $(SHAREDTARGET) obj/shareddbg/*/*.o -lX11 -lgcc -lc

dist: clean include depend static shared
	-$(RM) -rf $(DISTDIR)
	find /local/qt/examples -type d -print \
	    | grep -v RCS \
	    | sed s@/local@$(DISTDIR)/lib@ \
	    | xargs mkdir -p
	mkdir -p $(DISTDIR)/{lib/qt/include,lib/qt/doc,bin,man}
	cp -fR /local/qt/install-extra/. $(DISTDIR)
	cp -f $(INCDIR)/*.h $(DISTDIR)/lib/qt/include
	cp -f /local/qt/doc/html/*.html $(DISTDIR)/lib/qt/doc
	set -e ; for a in `find $(DISTDIR)/lib/qt/examples -type d -print` ; \
	do \
	    ( set -e ; cd $$a ; co -r `pwd | sed s@$(DISTDIR)/lib@/local@`/RCS/*,v ); \
	done

tar: dist
	-rm $(DISTDIR).tar.gz
	( set -e ; cd $(DISTDIR) ; tar cf misc.tar bin lib man )
	( set -e ; cd $(LIBDIR) ; tar cf $(DISTDIR)/shared.tar libqt.so* )
	( set -e ; cd $(LIBDIR) ; tar cf $(DISTDIR)/static.tar libqt.a )
	( set -e ; cd $(DISTDIR)/.. ; tar cf $(DISTDIR).tar `basename $(DISTDIR)`/{INSTALL,README,ANNOUNCE,MANIFEST,LICENCE,misc.tar,static.tar,shared.tar} ; gzip -f9 $(DISTDIR).tar )

info:
	@for d in $(DIRS); do cd $$d ; rinfo ; cd .. ; done

newer:
	@for d in $(DIRS); do cd $$d ; rnewer ; cd .. ; done

update:
	@for d in $(DIRS); do cd $$d ; rupdate ; cd .. ; done

checkout: 
	set -e ; for d in $(DIRS); do make -C $$d checkout ; done

showdirs:
	@echo $(DIRS)

showfiles:
	set -e ; for d in $(DIRS); do make -C $$d showfiles ; done

showsources:
	set -e ; for d in $(DIRS); do make -C $$d showsources ; done

showheaders:
	set -e ; for d in $(DIRS); do make -C $$d showheaders" ; done

clean:
	set -e ; for d in $(DIRS); do make -C $$d clean ; done

depend dep:
	set -e ; for d in $(DIRS); do make -C $$d depend ; done

include:
	-rm $(INCDIR)/*.h
	set -e ; for d in $(DIRS); do make -C $$d include ; done

tags:
	echo */*.cpp */*.h | fmt -1 | egrep -v '_win|_os2|moc/' | etags --c++ -
