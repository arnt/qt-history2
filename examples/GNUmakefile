# $Source: /tmp/cvs/qt/examples/GNUmakefile,v $
#

####### Tools

TMAKE	=	tmake

####### Directories

SUBDIRS =	aclock \
		addressbook \
		application \
		biff \
		buttons_groups \
		checklists \
		cursor \
		customlayout \
		dclock \
		desktop \
		dirview \
		dragdrop \
		drawdemo \
		drawlines \
		forever \
		hello \
		i18n \
		layout \
		life \
		lineedits \
		listbox \
		listbox_combo \
		listviews \
		menu \
		movies \
		picture \
		popup \
		progress \
		progressbar \
		qbrowser \
		qdir \
		qfd \
		qfileiconview \
		qiconview \
		qmag \
		qwerty \
		rangecontrols \
		richtext \
		rot13 \
		scribble \
		scrollview \
		showimg \
		splitter \
		tabdialog \
		table \
		tetrix \
		themes \
		tictac \
		tooltip \
		validator \
		widgets \
		wizard \
		xform

####### Targets

all: $(SUBDIRS)

showdirs:
	@echo $(SUBDIRS)

$(SUBDIRS): FORCE
	cd $@ && $(MAKE)

galore:
	for a in */* ; do [ -f $$a -a -x $$a ] && ( cd `dirname $$a` ; ../$$a & ) ; done

tmake:
	for i in $(SUBDIRS); do ( cd $$i  && $(TMAKE) $$i.pro -o Makefile ) ; done

clean:
	for i in $(SUBDIRS); do ( cd $$i  && $(MAKE) clean ) ; done

Makefile: examples.t examples.pro
	$(TMAKE) examples.pro -o Makefile

FORCE:
