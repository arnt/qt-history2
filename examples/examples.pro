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
		demo \
		dirview \
		dragdrop \
		drawdemo \
		drawlines \
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
		progress \
		progressbar \
		qdir \
		qfd \
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
		tetrix \
		themes \
		tictac \
		tooltip \
		validator \
		widgets \
		wizard \
		xform

canvas:SUBDIRS +=   canvas
opengl:SUBDIRS +=   box \
		    gear \
		    glpixmap \
		    overlay \
		    sharedbox \
		    texture
nas:SUBDIRS += 	    sound
iconview:SUBDIRS += fileiconview \
		    iconview
network:SUBDIRS +=  ftpclient \
		    httpd \
		    mail \
		    networkprotocol
workspace:SUBDIRS+= mdi
table:SUBDIRS +=    statistics \
		    table
sql:SUBDIRS += sql

embedded:SUBDIRS += winmanager \
		notepad \
		kiosk \
		launcher

embedded:SUBDIRS -= showimg

X11DIRS	    =   biff \
		desktop
