TEMPLATE    =	subdirs

SUBDIRS	=	aclock \
		action \
		action/actiongroup \
		action/toggleaction \
		addressbook \
		application \
		buttongroups \
		checklists \
		cursor \
		customlayout \
		dclock \
		demo \
		dirview \
		dragdrop \
		drawdemo \
		drawlines \
		fonts \
		forever \
		gridview \
		hello \
		helpviewer \
		i18n \
		layout \
		life \
		lineedits \
		listboxcombo \
		listbox \
		listviews \
		menu \
		movies \
		picture \
		popup \
		process \
		progress \
		progressbar \
		qdir \
		qfd \
		qmag \
		qtl \
		qwerty \
		rangecontrols \
		richtext \
		rot13 \
		scribble \
		scrollview \
		showimg \
		splitter \
		tabdialog \
		tetrix \
		textedit \
		themes \
		tictac \
		tooltip \
		tux \
		widgets \
		wizard \
		xform

thread:SUBDIRS +=   thread/semaphores

canvas:SUBDIRS +=   canvas

opengl:SUBDIRS +=   opengl/box \
		    opengl/gear \
		    opengl/glpixmap \
		    opengl/overlay \
		    opengl/sharedbox \
		    opengl/texture

nas:SUBDIRS += 	    sound

iconview:SUBDIRS += fileiconview \
		    iconview \
		    iconview/simple_dd
				

network:SUBDIRS +=  network/clientserver/client \
		    network/clientserver/server \
		    network/ftpclient \
		    network/httpd \
		    network/mail \
		    network/networkprotocol

workspace:SUBDIRS+= mdi

table:SUBDIRS +=    table/statistics \
		    table/small-table-demo \
		    table/bigtable

tablet:SUBDIRS += tablet

sql:SUBDIRS += sql

xml:SUBDIRS +=	xml/outliner \
		xml/tagreader \
		xml/tagreader-with-features

embedded:SUBDIRS += launcher

embedded:SUBDIRS -= showimg

win32:SUBDIRS += trayicon

X11DIRS	    =   biff \
		desktop
