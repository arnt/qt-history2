# -*- makefile -*-
#
# Main Makefile for building the Qt library, examples and tutorial.

SHELL=/bin/sh

init: FORCE
	@$(MAKE) all

install: FORCE
	@$(MAKE) qt.install

all: symlinks src-qmake src-moc sub-src sub-tools sub-tutorial sub-examples
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
	cd src/moc && [ -f Makefile ] && $(MAKE) install

src-install: sub-src
	cd src && [ -f Makefile ] && $(MAKE) install

tools-install: sub-tools
	cd tools && [ -f Makefile ] && $(MAKE) install

qmake-install: src-qmake
	cd qmake && [ -f Makefile ] && $(MAKE) install

src-qmake: symlinks FORCE
	cd qmake && [ -f Makefile ] && $(MAKE)

src-moc: src-qmake FORCE
	cd src/moc && [ -f Makefile ] && $(MAKE)

sub-tools: sub-plugins FORCE
	cd tools && [ -f Makefile ] && $(MAKE)

symlinks: .qmake.cache
#	@cd include && rm -f q*.h; ln -s ../src/*/q*.h .; ln -s ../extensions/*/src/q*.h .; rm -f q*_p.h

sub-src: src-moc .qmake.cache FORCE
	cd src && [ -f Makefile ] && $(MAKE)

sub-plugins: sub-src .qmake.cache FORCE
	cd plugins/src && [ -f Makefile ] && $(MAKE)

sub-tutorial: sub-src FORCE
	cd tutorial && [ -f Makefile ] && $(MAKE)

sub-examples: sub-tools FORCE
	cd examples && [ -f Makefile ] && $(MAKE)

clean uiclean mocclean:
	cd qmake && [ -f Makefile ] && $(MAKE) $@
	cd tools && [ -f Makefile ] && $(MAKE) $@
	cd src/moc && [ -f Makefile ] && $(MAKE) $@
	cd src && [ -f Makefile ] && $(MAKE) $@
	cd tutorial && [ -f Makefile ] && $(MAKE) $@
	cd plugins/src && [ -f Makefile ] && $(MAKE) $@
	cd examples && [ -f Makefile ] && $(MAKE) $@

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
