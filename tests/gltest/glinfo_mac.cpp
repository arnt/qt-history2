/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "glinfo.h"

#include "qstring.h"
#include <qlistview.h>
#include <qlayout.h>
#include <qtextview.h>
#include <qpushbutton.h>

#include <agl.h>
#include <gl.h>


GLInfo::GLInfo(QWidget* parent, const char* name)
    : QDialog(parent, name)
{
    glw = new QGLWidget( this );
    infotext = new QString("GLTest:\n");
    viewlist = new QStringList();
 
    QVBoxLayout *layout = new QVBoxLayout(this);
    infoView = new QTextView( this, "infoView" );
    layout->addWidget( infoView, 7 );
 
    infoList = new QListView( this, "infoList" );
    infoList->addColumn( trUtf8( "Nr", "" ) );
    infoList->addColumn( trUtf8( "Colorbits", "" ) );
    infoList->addColumn( trUtf8( "Draw to", "" ) );
    infoList->addColumn( trUtf8( "Transparent", "" ) );
    infoList->addColumn( trUtf8( "buff size", "" ) );
    infoList->addColumn( trUtf8( "level", "" ) );
    infoList->addColumn( trUtf8( "render type", "" ) );
    infoList->addColumn( trUtf8( "DB", "" ) );
    infoList->addColumn( trUtf8( "stereo", "" ) );
    infoList->addColumn( trUtf8( "R sz", "" ) );
    infoList->addColumn( trUtf8( "G sz", "" ) );
    infoList->addColumn( trUtf8( "B sz", "" ) );
    infoList->addColumn( trUtf8( "A sz", "" ) );
    infoList->addColumn( trUtf8( "aux buff", "" ) );
    infoList->addColumn( trUtf8( "depth", "" ) );
    infoList->addColumn( trUtf8( "stencil", "" ) );
    infoList->addColumn( trUtf8( "R accum", "" ) );
    infoList->addColumn( trUtf8( "G accum", "" ) );
    infoList->addColumn( trUtf8( "B accum", "" ) );
    infoList->addColumn( trUtf8( "A accum", "" ) );
    infoList->addColumn( trUtf8( "MS num", "" ) );
    infoList->addColumn( trUtf8( "MS bufs", "" ) );
    infoList->setSelectionMode( QListView::Extended );
    infoList->setAllColumnsShowFocus( TRUE );
    layout->addWidget( infoList, 10 );
    
    QHBoxLayout *buttonLayout = new QHBoxLayout( this );
    layout->addLayout( buttonLayout );
    buttonLayout->addStretch();
    
    QPushButton *ok = new QPushButton( "Ok", this );
    connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ));
    buttonLayout->addWidget( ok );
    
    int i;
    QListViewItem *item;
    QStringList list, listItem;
    infoView->setText( getText() );
    for ( QStringList::Iterator it = viewlist->begin(); it != viewlist->end(); ++it ) {
        i = 0;
        item = new QListViewItem(infoList);
        listItem = QStringList::split(" ", (*it).latin1());
        for ( QStringList::Iterator ti = listItem.begin(); ti != listItem.end(); ++ti ) {
            item->setText(i, (*ti).latin1());
            i++;
        }
        infoList->insertItem(item);
    }
};

QString GLInfo::getText()
{
    int   i;
    char* s;
    char  t[80];
    char* p;
    
    makeCurrent();
    return *infotext;

    infotext->sprintf("%sdisplay: N/A\n"
		      "server agl vendor string: N/A\n"
		      "server agl version string: N/A\n"
		      "server agl extensions (AGL_): N/A\n"
		      "client agl version: N/A\n"
		      "client agl extensions (AGL_): none\n"
		      "OpenGL vendor string: %s\n", (const char*)*infotext, 
		      ((const char*)glGetString(GL_VENDOR)));
    infotext->sprintf("%sOpenGL renderer string: %s\n", (const char*)*infotext, 
		      glGetString(GL_RENDERER));
    infotext->sprintf("%sOpenGL version string: %s\n", (const char*)*infotext, 
		      glGetString(GL_VERSION));
    infotext->sprintf("%sOpenGL extensions (GL_): \n", (const char*)*infotext);
    i = 0;
    s = (char*)glGetString(GL_EXTENSIONS);
	t[79] = '\0';
    while(*s) {
		t[i++] = *s;
		if(*s == ' ') {
			if (*(s+1) != '\0') {
				t[i-1] = ',';
				t[i] = ' ';
				p = &t[i++];
			} else {       // zoinks! last one terminated in a space! //
				t[i-1] = '\0';
			}
		}
		if(i > 80 - 5) {
			*p = t[i] = '\0';
			infotext->sprintf("%s    %s\n", (const char*)*infotext, t);
			p++;
			i = strlen(p);
			strcpy(t, p);
		}
		s++;
	}
    t[i] = '\0';
    infotext->sprintf("%s    %s.", (const char*)*infotext, t);
}

QStringList GLInfo::getViewList()
{
  return *viewlist;
}
