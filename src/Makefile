#############################################################################
# Qt Global Makefile
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2 -DCHECK_MEMORY
CDBGFLAGS	= -g -DCHECK_MEMORY
AOUTCFLAGS	= -b i486-linuxaout
ELFCFLAGS	= -fPIC
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
	@exit 1

#
# Create the tools library (necessary for the meta object compiler)
#

tools:
	cd tools
	make

#
# Create the meta object compiler
#

moc:
	cd tools
	make
	cd moc
	make

#
# Create a library, called from aout, aoutdbg, elf and elfdbg
#

lib:
	$(RM) -f tools/*.o
	$(RM) -f kernel/*.o
	$(RM) -f widgets/*.o
	$(MKDIR) -p $(OBJDIR)/tools
	$(MKDIR) -p $(OBJDIR)/kernel
	$(MKDIR) -p $(OBJDIR)/widgets
	-$(MV) -f $(OBJDIR)/tools/*.o tools
	( cd tools; make obj 'CFLAGS=$(CFLAGS)' )
	-$(MV) -f tools/*.o $(OBJDIR)/tools
	-$(MV) -f $(OBJDIR)/kernel/*.o kernel
	( cd kernel; make obj 'CFLAGS=$(CFLAGS)' )
	-$(MV) -f kernel/*.o $(OBJDIR)/kernel	
	-$(MV) -f $(OBJDIR)/widgets/*.o widgets
	( cd widgets; make obj 'CFLAGS=$(CFLAGS)' )
	-$(MV) -f widgets/*.o $(OBJDIR)/widgets

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

checkout: 
	@for d in $(DIRS); \
	do (cd $$d; echo "[--- $$d ---]"; \
	    make checkout; \
	) done

clean:
	@for d in $(DIRS); \
	do (cd $$d; echo "[--- $$d ---]"; \
	    make clean \
	) done

depend:
	@for d in $(DIRS); \
	do (cd $$d; echo "[--- $$d ---]"; \
	    make clean \
	) done
