# $Source: /tmp/cvs/qt/examples/GNUmakefile,v $
#

####### Tools

TMAKE	=	tmake

####### Directories

SUBDIRS =	application \
		buttons_groups \
		checklists \
		cursor \
		customlayout \
		desktop \
		dirview \
		dragdrop \
		drawdemo \
		drawlines \
		forever \
		hello \
		i18n \
		layout \
		linedits \
		listbox_combo \
		listviews \
		menu \
		picture \
		popup \
		progress \
		progressbar \
		qiconview \
		rangecontrols \
		richtext \
		rot13 \
		scrollview \
		splitter \
		tabdialog \
		table \
		themes \
		tooltip \
		validator \
		wizard \
		xform

####### Targets

all: $(SUBDIRS)

showdirs:
	@echo $(SUBDIRS)

$(SUBDIRS): FORCE
	cd $@; $(MAKE)

galore:
	for a in */* ; do [ -f $$a -a -x $$a ] && ( cd `dirname $$a` ; ../$$a & ) ; done

tmake:
	for i in $(SUBDIRS); do ( cd $$i ; $(TMAKE) $$i.pro -o Makefile ) ; done

clean depend:
	for i in $(SUBDIRS); do ( cd $$i ; $(MAKE) $@ ) ; done

Makefile: simple.t simple.pro
	$(TMAKE) simple.pro -o Makefile

FORCE:
