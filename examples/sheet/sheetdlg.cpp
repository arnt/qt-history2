/****************************************************************************
** $Id: //depot/qt/main/examples/sheet/sheetdlg.cpp#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qmenubar.h>
#include <qkeycode.h>


#include "sheet.h"
#include "sheetdlg.h"

MyWidget::MyWidget( QWidget *parent, const char *name )
    :QWidget( parent, name )
{
    t = new Sheet(this); 
    t->move(10,50);

    QMenuBar* m = new QMenuBar( this );

    QPopupMenu* p1 = new QPopupMenu;

    int id = p1->insertItem("New" );
    p1->setItemEnabled( id, FALSE );
    id = p1->insertItem("Open..." );
    p1->setItemEnabled( id, FALSE );
    id = p1->insertItem("Save" );
    p1->setItemEnabled( id, FALSE );
    id = p1->insertItem("Save As..." );
    p1->setItemEnabled( id, FALSE );
    p1->insertItem("Quit", qApp , SLOT(quit()) );
    m->insertItem("File", p1);

    QPopupMenu* p2 = new QPopupMenu;
    m->insertItem("Chart", p2 );
    p2->insertItem("Show Pie Chart", t , SLOT(showPie()) );
    p2->insertItem("Hide Pie Chart", t , SLOT(hidePie()) );


    resizeHandle( size() );

}


void MyWidget::resizeEvent( QResizeEvent * e )
{
    resizeHandle( e->size() );
}

void MyWidget::resizeHandle( QSize s )
{
    t->resize(s.width()-20, s.height()-70);
}
