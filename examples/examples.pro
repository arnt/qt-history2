TEMPLATE    =	subdirs
SUBDIRS     =	aclock \
		action \
		addressbook \
		application \
		buttongroups \
		checklists \
		cursor \
		customlayout \
		dclock \
		dirview \
		dragdrop \
		drawdemo \
		drawlines \
		fonts \
		forever \
		guithreads \
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
		semaphores \
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
		validator \
		widgets \
		wizard \
		xform
!contains(QT_PRODUCT,qt-professional): SUBDIRS += demo

canvas:SUBDIRS +=   canvas
opengl:SUBDIRS +=   opengl/box \
		    opengl/gear \
		    opengl/glpixmap \
		    opengl/overlay \
		    opengl/sharedbox \
		    opengl/texture
nas:SUBDIRS += 	    sound
iconview:SUBDIRS += fileiconview \
		    iconview
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
sql:SUBDIRS += sql

xml:SUBDIRS +=	xml/outliner \
		xml/tagreader \
		xml/tagreader-with-features

embedded:SUBDIRS += winmanager \
		notepad \
		kiosk \
		launcher

embedded:SUBDIRS -= showimg

win32:SUBDIRS += trayicon

X11DIRS	    =   biff \
		desktop
