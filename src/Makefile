#############################################################################
# Qt Global Makefile
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -W -Wtemplate-debugging -Wparentheses -Wuninitialized \
		-Wchar-subscripts -Wformat -Wtrigraphs -Wcomment -Wswitch \
		-Wunused -Wreturn-type -Wimplicit -Wpointer-arith -Wsynth \
		-Wconversion -Wno-overloaded-virtual \
		-O2 -fno-strength-reduce
CDBGFLAGS	= -g -W -Wall -Wpointer-arith -Wsynth -Wconversion
CPROFLAGS	= -pg -O2 -fno-strength-reduce
CSTATICFLAGS	=
CSHAREDFLAGS	= -fPIC
CC		= gcc

####### Targets

STATICTARGET	= $(LIBDIR)/libqt.a
SHAREDTARGET	= $(LIBDIR)/libqt.so.1.3

#######	Sub directories (excluding moc)

DIRS		= tools kernel widgets dialogs

#######	Build rules

all:
	@echo "PLEASE SPECIFY A TARGET"
	@echo "  make static    - creates a static library"
	@echo "  make staticdbg - creates a static library w/debug info"
	@echo "  make staticpro - creates a static library w/profiling code"
	@echo "  make shared    - creates a shared library"
	@echo "  make shareddbg - creates a shared library w/debug info"
	@echo "  make moc       - creates the meta object compiler"
	@echo "  make tools     - creates the tools library"
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
tools kernel widgets dialogs: dodepend FORCE
	-mkdir -p $(OBJDIR)/$@
	-rm -f $@/*.o
	-ln -f $(OBJDIR)/$@/*.o $@
	$(MAKE) -C $@ obj
	rm -f $(OBJDIR)/$@/*.o
	mv -f $@/*.o $(OBJDIR)/$@

comeau:
	$(MAKE) lib CC=como CFLAGS="--strict_warnings -a" OBJDIR=obj/comeau

static:
	/bin/rm -f $(STATICTARGET)
	$(MAKE) lib 'CC=$(CC)' 'CFLAGS=$(CFLAGS) $(CSTATICFLAGS)' 'OBJDIR=obj/static'
	ar q $(STATICTARGET) obj/static/*/*.o
	ranlib $(STATICTARGET)

staticdbg:
	/bin/rm -f $(STATICTARGET)
	$(MAKE) lib 'CC=$(CC)' 'CFLAGS=$(CDBGFLAGS) $(CSTATICFLAGS)' 'OBJDIR=obj/staticdbg'
	ar q $(STATICTARGET) obj/staticdbg/*/*.o
	ranlib $(STATICTARGET)

staticpro:
	$(MAKE) lib 'CC=$(CC)' 'CFLAGS=$(CPROFLAGS) $(CSTATICFLAGS)' 'OBJDIR=obj/staticpro'
	/bin/rm -f $(STATICTARGET)
	ar q $(STATICTARGET) obj/staticpro/*/*.o
	ranlib $(STATICTARGET)

shared:
	/bin/rm -f $(SHAREDTARGET)
	$(MAKE) lib 'CC=$(CC)' 'CFLAGS=$(CFLAGS) $(CSHAREDFLAGS)' 'OBJDIR=obj/shared'
	$(CC) -shared -Wl,-soname,libqt.so.1 -o $(SHAREDTARGET) \
		obj/shared/*/*.o -L/usr/X11R6/lib -lX11 -ldl -lc -lgcc

shareddbg:
	/bin/rm -f $(SHAREDTARGET)
	$(MAKE) lib 'CC=$(CC)' 'CFLAGS=$(CDBGFLAGS) $(CSHAREDFLAGS)' 'OBJDIR=obj/shareddbg'
	$(CC) -shared -Wl,-soname,libqt.so.1 -o $(SHAREDTARGET) \
		obj/shareddbg/*/*.o -L/usr/X11R6/lib -lX11 -ldl -lc -lgcc

showdirs:
	@echo $(DIRS)

showfiles showheaders showsources showinternal:
	@set -e ; for a in $(DIRS) ; do $(MAKE) -s -C $$a $@ | fmt -1 | sed 's-^-'$${a}/- ; done

dodepend:
	$(MAKE) include
	$(MAKE) depend
	touch dodepend

depend:
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d depend ; done

include:
	-rm -f $(INCDIR)/*.h
	( cd $(INCDIR) ; \
	  ln -s -f ../src/tools/*.h . ; \
	  ln -s -f ../src/kernel/*.h . ; \
	  ln -s -f ../src/widgets/*.h . ; \
	  ln -s -f ../src/dialogs/*.h . )

clean:
	-rm -f dodepend
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d $@ ; done

tags:
	echo */*.cpp */*.h | fmt -1 | egrep -v '_win|_os2|moc/' | etags --c++ -
