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
ELFTARGET	= $(LIBDIR)/libqt.so.0.9.0
ELFTARGETPRE	= libqt.so.0

####### Misc programs used

RM		= /bin/rm
MV		= /bin/mv
MKDIR		= /bin/mkdir

#######	Sub directories

DIRS		= tools moc kernel widgets

#######	Build rules

all:
	@echo "PLEASE SPECIFY A TARGET"
	@echo "  make aout    - creates an aout static library"
	@echo "  make aoutdbg - creates an aout static library w/debug info"
	@echo "  make elf     - creates an ELF dynamic library"
	@echo "  make elfdbg  - creates an ELF dynamic library w/debug info"
	@echo "  make moc     - creates the meta object compiler"
	@echo "  make tools   - creates the tools library"
	@echo "  make dist    - make libraries for a distribution"
	@exit 1

#
# Create the tools library (necessary for the meta object compiler)
#

tools: toolshack

toolshack:
	make -C tools

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

lib:
	$(RM) -f tools/*.o
	$(RM) -f kernel/*.o
	$(RM) -f widgets/*.o
	$(MKDIR) -p $(OBJDIR)/tools
	$(MKDIR) -p $(OBJDIR)/kernel
	$(MKDIR) -p $(OBJDIR)/widgets
	-$(MV) -f $(OBJDIR)/tools/*.o tools
	make -C tools obj 'CFLAGS=$(CFLAGS)'
	$(MV) -f tools/*.o $(OBJDIR)/tools
	-$(MV) -f $(OBJDIR)/kernel/*.o kernel
	make -C kernel obj 'CFLAGS=$(CFLAGS)'
	$(MV) -f kernel/*.o $(OBJDIR)/kernel	
	-$(MV) -f $(OBJDIR)/widgets/*.o widgets
	make -C widgets obj 'CFLAGS=$(CFLAGS)'
	$(MV) -f widgets/*.o $(OBJDIR)/widgets

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
	$(CC) -shared -Wl,-soname -Wl,$(ELFTARGETPRE) \
		-o $(ELFTARGET) obj/elf/*/*.o -lX11 -lgcc -lc

elfdbg:
	make lib 'CFLAGS=$(CDBGFLAGS) $(ELFCFLAGS)' 'OBJDIR=obj/elfdbg'
	/bin/rm -f $(ELFTARGET)
	$(CC) -shared -Wl,-soname -Wl,$(ELFTARGETPRE) \
		-o $(ELFTARGET) obj/elfdbg/*/*.o -lX11 -lgcc -lc

dist: clean include depend
	make lib 'CFLAGS=-DLINUX_RESTRICTED $(CFLAGS) $(ELFCFLAGS)' 'OBJDIR=obj/elf'
	/bin/rm -f $(ELFTARGET)
	$(CC) -shared -Wl,-soname -Wl,$(ELFTARGETPRE) \
		-o $(ELFTARGET) obj/elfdbg/*/*.o -lX11 -lgcc -lc
	make lib 'CFLAGS=-DLINUX_RESTRICTED $(CFLAGS) $(AOUTCFLAGS)' 'OBJDIR=obj/aout'
	/bin/rm -f $(AOUTTARGET)
	ar q $(AOUTTARGET) obj/aout/*/*.o
	ranlib $(AOUTTARGET)
	-$(RM) -rf $(DISTDIR)
	find /local/qt/examples -type d -print \
	    | grep -v RCS \
	    | sed s@/local@$(DISTDIR)/lib@ \
	    | xargs mkdir -p
	$(MKDIR) -p $(DISTDIR)/{lib/qt/include,lib/qt/doc,bin,man}
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

checkout: 
	set -e ; for d in $(DIRS); do make -C $$d checkout ; done

clean:
	set -e ; for d in $(DIRS); do make -C $$d clean ; done

depend dep:
	set -e ; for d in $(DIRS); do make -C $$d depend ; done

include:
	-rm $(INCDIR)/*.h
	set -e ; for d in $(DIRS); do make -C $$d include ; done
