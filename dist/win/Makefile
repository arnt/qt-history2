# -*- makefile -*-
#
# Main Makefile for building the Qt library, examples and tutorial.
# Read PORTING for instructions how to port Qt to a new platform.

all: symlinks sub-src sub-tools sub-tutorial sub-examples
	@echo .
	@echo The Qt library is now built in .\lib
	@echo The Qt examples are built in the directories in .\examples
	@echo The Qt tutorials are built in the directories in .\tutorial
	@echo .
	@echo Note: be sure to set %%QTDIR%% to point to here or to wherever
	@echo       you move these directories.
	@echo .
	@echo Enjoy!   - the Trolltech team
	@echo .

src-qmake: symlinks
	cd qmake
	$(MAKE)
	cd ..

src-moc: src-qmake FORCE
	cd src\moc
	$(MAKE)
	cd ..\..

sub-tools: sub-src FORCE
	cd tools
	$(MAKE)
	cd ..

symlinks: .qmake.cache
#	syncqt

sub-src: .qmake.cache FORCE
	cd src
	$(MAKE)
	$(MAKE) -f Makefile.main
	cd ..

sub-tutorial: sub-src FORCE
	cd tutorial
	$(MAKE)
	cd ..

sub-examples: sub-src FORCE
	cd examples
	$(MAKE)
	cd ..

clean:
	cd qmake&&$(MAKE) clean
	cd tools&&$(MAKE) clean
	cd src\moc&&$(MAKE) clean
	cd src&&$(MAKE) clean&&$(MAKE) clean -f Makefile.main
	cd tutorial&&$(MAKE) clean
	cd examples&&$(MAKE) clean

distclean: clean
	-del .qmake.cache

.qmake.cache: 
	@echo
	@echo '  Qt must first be configured using the "configure" script.'
	@echo

FORCE: