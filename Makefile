# -*- makefile -*-
#
# Main Makefile for building the Qt library, examples and tutorial.
# Read PORTING for instructions how to port Qt to a new platform.

!IF "$(MAKE)" == "NMAKE" || "$(MAKE)" == "Nmake" || "$(MAKE)" == "nmake"
QMFILE=Makefile
FORCEDEP=FORCE
CONTINUEONERROR=/K
!ELSE
QMFILE=Makefile.borland
FORCEDEP=
CONTINUEONERROR= ###Something for Rainer to do
!ENDIF

all: symlinks src-qmake sub-src sub-tools sub-tutorial sub-examples
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
	$(MAKE) -f $(QMFILE)
	cd ..

src-moc: src-qmake $(FORCEDEP)
	cd src\moc
	$(MAKE)
	cd ..\..

sub-tools: sub-codecs $(FORCEDEP)
	cd tools
	$(MAKE)
	cd ..

symlinks: .qmake.cache
#	syncqt

sub-src: .qmake.cache $(FORCEDEP)
	cd src
	$(MAKE)
	$(MAKE) -f Makefile.main
	cd ..

sub-codecs: sub-src .qmake.cache $(FORCEDEP)
	cd src\codecs\src
	$(MAKE)
	cd ..\..\..

sub-tutorial: sub-src $(FORCEDEP)
	cd tutorial
	$(MAKE) $(CONTINUEONERROR)
	cd ..

sub-examples: sub-src $(FORCEDEP)
	cd examples
	$(MAKE) $(CONTINUEONERROR)
	cd ..

clean:
	cd qmake
	$(MAKE) -f $(QMFILE) $(CONTINUEONERROR) clean
	cd ..\tools
	$(MAKE) $(CONTINUEONERROR) clean
	cd ..\src
	$(MAKE) $(CONTINUEONERROR) clean
	$(MAKE) -f Makefile.main $(CONTINUEONERROR) clean
	cd ..\tutorial
	$(MAKE) $(CONTINUEONERROR) clean
	cd ..\examples
	$(MAKE) $(CONTINUEONERROR) clean

distclean: clean
	-del .qmake.cache

.qmake.cache: 
	@echo
	@echo '  Qt must first be configured using the "configure" script.'
	@echo

FORCE:
