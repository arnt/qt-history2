# -*- makefile -*-
#
# Main Makefile for building the Qt library, examples and tutorial.
# Read PORTING for instructions how to port Qt to a new platform.

SHELL=/bin/sh

init: FORCE
	@$(MAKE) QTDIR=`pwd` all

install: FORCE
	@$(MAKE) QTDIR=`pwd` qt.install

all: symlinks src-qmake src-moc sub-src sub-tools \
		sub-tutorial sub-examples
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

qt.install: qmake-install moc-install src-install tools-install

moc-install: src-moc
	cd src/moc && $(MAKE) install

src-install: sub-src
	cd src && $(MAKE) install

tools-install: sub-tools
	cd tools && $(MAKE) install

qmake-install: src-qmake
	cd qmake && $(MAKE) install

src-qmake: symlinks FORCE
	cd qmake && $(MAKE)

src-moc: src-qmake FORCE
	cd src/moc && $(MAKE)

sub-tools: sub-plugins FORCE
	cd tools && $(MAKE)

symlinks: .qmake.cache
#	@cd include && rm -f q*.h; ln -s ../src/*/q*.h .; ln -s ../extensions/*/src/q*.h .; rm -f q*_p.h

sub-src: src-moc .qmake.cache FORCE
	cd src && $(MAKE)

sub-plugins: sub-src .qmake.cache FORCE
	cd plugins/src && $(MAKE)

sub-tutorial: sub-src FORCE
	cd tutorial && $(MAKE)

sub-examples: sub-tools FORCE
	cd examples && $(MAKE)

clean uiclean mocclean:
	cd qmake && $(MAKE) $@
	cd tools && $(MAKE) $@
	cd src/moc && $(MAKE) $@
	cd src && $(MAKE) $@
	cd tutorial && $(MAKE) $@
	cd plugins/src && $(MAKE) $@
	cd examples && $(MAKE) $@

distclean: clean
	-rm .qmake.cache

doc: FORCE
	qdoc util/qdoc/qdoc.conf

.qmake.cache:
	@echo
	@echo '  Qt must first be configured using the "configure" script.'
	@echo
	@test ! /bin/true

FORCE:


