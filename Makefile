# -*- makefile -*-
#
# Main Makefile for building the Qt library, examples and tutorial.
# Read PORTING for instructions how to port Qt to a new platform.

SHELL=/bin/sh

init: FORCE
	@$(MAKE) QTDIR=`pwd` all

all: qmake symlinks src-moc sub-src sub-tools sub-tutorial sub-examples
	@echo
	@echo "The Qt library is now built in ./lib"
	@echo "The Qt examples are built in the directories in ./examples"
	@echo "The Qt tutorials are built in the directories in ./tutorial"
	@echo
	@echo 'Note: be sure to set $$QTDIR to point to here or to wherever'
	@echo '      you move these directories.'
	@echo
	@echo "Enjoy!   - the Trolltech team"
	@echo

qmake: symlinks FORCE
	cd qmake; $(MAKE)
	-rm -f bin/qmake
	cp qmake/qmake bin/qmake

src-moc: .qmake.cache symlinks  FORCE
	cd src/moc; $(MAKE)
	-rm -f bin/moc
	cp src/moc/moc bin/moc

sub-tools: sub-src FORCE
	cd tools; $(MAKE)

symlinks: .qmake.cache
	@cd include; rm -f q*.h; ln -s ../src/*/q*.h .; ln -s ../extensions/*/src/q*.h .; rm -f q*_p.h

sub-src: src-moc .qmake.cache FORCE
	cd src; $(MAKE)

sub-tutorial: sub-src FORCE
	cd tutorial; $(MAKE)

sub-examples: sub-src FORCE
	cd examples; $(MAKE)

clean:
	cd qmake; $(MAKE) clean
	cd tools; $(MAKE) clean
	cd src/moc; $(MAKE) clean
	cd src; $(MAKE) clean
	cd tutorial; $(MAKE) clean
	cd examples; $(MAKE) clean

distclean: clean
	-rm .qmake.cache

.qmake.cache: 
	@echo
	@echo '  Qt must first be configured using the "configure" script.'
	@echo

FORCE:


