#############################################################################
# Qt Global Makefile
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib
DISTDIR		= /tmp/qt-0.91

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2 -DCHECK_MEMORY
CDBGFLAGS	= -g -DCHECK_MEMORY
AOUTCFLAGS	= -b i486-linuxaout
ELFCFLAGS	= -fPIC -b i486-linux
CC		= gcc

####### Targets

AOUTTARGET	= $(LIBDIR)/libqt.a
ELFTARGET	= $(LIBDIR)/libqt.so.0.9.2
ELFTARGETPRE	= libqt.so.0

#######	Sub directories
#
# most are also make targets

DIRS		= tools kernel widgets dialogs

#######	Build rules

all:
	@echo "PLEASE SPECIFY A TARGET"
	@echo "  make aout    - creates an aout static library"
	@echo "  make aoutdbg - creates an aout static library w/debug info"
	@echo "  make elf     - creates an ELF dynamic library"
	@echo "  make elf26   - creates an ELF dynamic library"
	@echo "  make elfdbg  - creates an ELF dynamic library w/debug info"
	@echo "  make moc     - creates the meta object compiler"
	@echo "  make tools   - creates the tools library"
	@echo "  make dist    - make libraries for a distribution"
	@exit 1

#
# Create the meta object compiler
#

moc: tools mochack

mochack:
	make -C moc

#
# Create a library, called from aout, aoutdbg, elf and elfdbg
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

aout:
	make lib 'CFLAGS=$(CFLAGS) $(AOUTCFLAGS)' 'OBJDIR=obj/aout'
	/bin/rm -f $(AOUTTARGET)
	ar q $(AOUTTARGET) obj/aout/*/*.o
	ranlib $(AOUTTARGET)

aoutdbg:
	make lib 'CFLAGS=$(CDBGFLAGS) $(AOUTCFLAGS)' 'OBJDIR=obj/aoutdbg'
	/bin/rm -f $(AOUTTARGET)
	ar q $(AOUTTARGET) obj/aoutdbg/*/*.o
	ranlib $(AOUTTARGET)

elf:
	make lib 'CFLAGS=$(CFLAGS) $(ELFCFLAGS)' 'OBJDIR=obj/elf'
	/bin/rm -f $(ELFTARGET)
	$(CC) -shared -o $(ELFTARGET) obj/elf/*/*.o -lX11 -lgcc -lc

elf26:
	make lib 'CFLAGS=$(CFLAGS) -V 2.6.3 $(ELFCFLAGS)' 'OBJDIR=obj/elf26'
	/bin/rm -f $(ELFTARGET)
	$(CC) -shared -o $(ELFTARGET) obj/elf26/*/*.o -lX11 -lgcc -lc

elfdbg:
	make lib 'CFLAGS=$(CDBGFLAGS) $(ELFCFLAGS)' 'OBJDIR=obj/elfdbg'
	/bin/rm -f $(ELFTARGET)
	$(CC) -shared -o $(ELFTARGET) obj/elfdbg/*/*.o -lX11 -lgcc -lc

dist: clean include depend
	make lib 'CFLAGS=-DLINUX_RESTRICTED $(CFLAGS) $(ELFCFLAGS)' 'OBJDIR=obj/elf'
	/bin/rm -f $(ELFTARGET)
	$(CC) -shared -o $(ELFTARGET) obj/elf/*/*.o -lX11 -lgcc -lc
	make lib 'CFLAGS=-DLINUX_RESTRICTED $(CFLAGS) $(AOUTCFLAGS)' 'OBJDIR=obj/aout'
	/bin/rm -f $(AOUTTARGET)
	ar q $(AOUTTARGET) obj/aout/*/*.o
	ranlib $(AOUTTARGET)
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
	( set -e ; cd $(LIBDIR) ; tar cf $(DISTDIR)/elf.tar libqt.so* )
	( set -e ; cd $(LIBDIR) ; tar cf $(DISTDIR)/aout.tar libqt.a )
	( set -e ; cd $(DISTDIR)/.. ; tar cf $(DISTDIR).tar `basename $(DISTDIR)`/{INSTALL,README,ANNOUNCE,MANIFEST,LICENCE,misc.tar,aout.tar,elf.tar} ; gzip -f9 $(DISTDIR).tar )

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
	echo */*.cpp */*.h | egrep -v '_win.cpp|_os2.cpp|moc/' | etags --c++ -
