#############################################################################
# Qt Global Makefile
#############################################################################

all:
	@echo "SPECIFY BUILD TARGET:"
	@echo "  make static    - creates a static library"
	@echo "  make staticdbg - creates a static library w/debug info"
	@echo "  make staticpro - creates a static library w/profiling code"
	@echo "  make shared    - creates a shared library"
	@echo "  make shareddbg - creates a shared library w/debug info"
	@echo "  make moc       - creates the meta object compiler"
	@echo
	@echo "MAINTENANCE:"
	@echo "  make include   - creates symlinks for the include files"
	@echo "  make clean     - removes temporary files"	
	@echo "  make tags      - updates the TAGS file"	
	@echo
	@echo "MORE INFORMATION:"
	@echo "  make help	- getting started with our make system"
	@echo "  make details	- more details about our make system"
	@echo

#
# Create the meta object compiler
#

moc: FORCE
	$(MAKE) -C moc
	cp moc/moc ../bin

#
# Create a library, called from static, staticdbg, shared and shareddbg
#

static: Makefile.static
	make -f Makefile.static

staticdbg: Makefile.staticdbg
	make -f Makefile.staticdbg

shared: Makefile.shared
	make -f Makefile.shared

shareddbg: Makefile.shareddbg
	make -f Makefile.shareddbg

Makefile.static: qtx11.pro qtinternal.pro Makefile
	mkdir -p obj/static
	tmake qtx11.pro -p qtinternal.pro -o Makefile.static "CONFIG+=staticlib" "OBJECTS_DIR=obj/static"

Makefile.staticdbg: qtx11.pro qtinternal.pro Makefile
	mkdir -p obj/staticdbg
	tmake qtx11.pro -p qtinternal.pro -o Makefile.staticdbg "CONFIG+=staticlib debug" "OBJECTS_DIR=obj/staticdbg"

Makefile.shared: qtx11.pro qtinternal.pro Makefile
	mkdir -p obj/shared
	tmake qtx11.pro -p qtinternal.pro -o Makefile.shared "OBJECTS_DIR=obj/shared"

Makefile.shareddbg: qtx11.pro qtinternal.pro Makefile
	mkdir -p obj/shareddbg
	tmake qtx11.pro -p qtinternal.pro -o Makefile.sharedebg "CONFIG+=debug" "OBJECTS_DIR=obj/shareddbg"


showdirs:
	@tmake qtx11.pro -p qtinternal.pro -e 'Expand("DIRS")'

showfiles:
	@tmake qtx11.pro -p qtinternal.pro -e 'Expand("HEADERS","SOURCES")'

showheaders:
	@tmake qtx11.pro -p qtinternal.pro -e 'Expand("HEADERS")'

showsources:
	@tmake qtx11.pro -p qtinternal.pro -e 'Expand("SOURCES")'

showinternal:
	@tmake qtx11.pro -p qtinternal.pro -e 'Expand("INTERNAL")'

showx11only:
	@tmake qtx11.pro -p qtinternal.pro -e 'Expand("X11ONLY")'


include:
	-rm -f $(INCDIR)/*.h
	( cd $(INCDIR) ; \
	  ln -s -f ../src/tools/*.h . ; \
	  ln -s -f ../src/kernel/*.h . ; \
	  ln -s -f ../src/widgets/*.h . ; \
	  ln -s -f ../src/dialogs/*.h . )

clean:
	-rm -f */moc_q*.cpp
	-rm -f Makefile.static Makefile.staticdbg
	-rm -f Makefile.shared Makefile.shareddbg


tags:
	@tmake qtx11.pro -p qtinternal.pro -e 'Expand("HEADERS","SOURCES")' \
		| fmt -1 | etags --c++ -


help:
	@echo "To use our make system, you must first install perl 5 or later"
	@echo "and the most recent version of tmake."
	@echo ""
	@echo "Make sure the QTDIR environment variable is correctly set, e.g:"
	@echo "    export QTDIR=\$$(HOME)/qt"
	@echo ""
	@echo "Make sure the TMAKEPATH environment variable is correctly set, e.g:"
	@echo "    export TMAKEPATH=\$$(HOME)/tmake/lib/linux-gcc:\$$(QTDIR)/tmake/linux-gcc"
	@echo ""
	@echo "Before building the library the first time, do the following:"
	@echo "    make include"
	@echo "    make moc"
	@echo ""
	@echo "To create the library, build one of the following targets:"
	@echo "    make shared		- shared library"
	@echo "    make shareddbg	- shared library with debug info"
	@echo "    make static		- static library"
	@echo "    make staticdbg	- static library with debug info"
	@echo ""
	@echo "If you are unhappy with the default compiler settings, you can"
	@echo "copy the \$$(HOME)/tmake/lib/linux-gcc/tmake.conf file to qt/src"
	@echo "and modify it to suit your needs."
	@echo ""

details:
	@echo "The file qtx11.pro contains a list of all public Qt files."
	@echo "You should not need to modify this file."
	@echo ""
	@echo "The file qtinternal.pro is an internal project file for Troll Tech."
	@echo "It lists files that we use internally.  If you want to add new"
	@echo "files, e.g. qcolordialog.h and .cpp, add them to the INT_HEADERS"
	@echo "and INT_SOURCES sections."


FORCE:
