/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>

#include "qvfb.h"
#include "qvfbview.h"
#include "qvfbratedlg.h"

QVFb::QVFb( int w, int h, int d, QWidget *parent, const char *name,
			uint flags )
    : QMainWindow( parent, name, flags )
{
    QString cap("Virtual framebuffer %1x%2 %3bpp");
    setCaption( cap.arg(w).arg(h).arg(d) );
    rateDlg = 0;
    view = new QVFbView( w, h, d, this );
    setCentralWidget( view );
    createMenu();
}

QVFb::~QVFb()
{
}

void QVFb::enableCursor( bool e )
{
    view->viewport()->setCursor( e ? ArrowCursor : BlankCursor );
    viewMenu->setItemChecked( cursorId, e );
}

void QVFb::createMenu()
{
    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( "&Quit", qApp, SLOT(quit()) );

    menuBar()->insertItem( "&File", file );

    viewMenu = new QPopupMenu( this );
    viewMenu->setCheckable( true );
    cursorId = viewMenu->insertItem( "Show &Cursor", this, SLOT(slotCursor()) );
    viewMenu->setItemChecked( cursorId, true );
    viewMenu->insertItem( "&Refresh Rate...", this, SLOT(slotRateDlg()) );

    menuBar()->insertItem( "&View", viewMenu );
}

void QVFb::slotCursor()
{
    enableCursor( !viewMenu->isItemChecked( cursorId ) );
}

void QVFb::slotRateDlg()
{
    if ( !rateDlg ) {
	rateDlg = new QVFbRateDialog( view->rate(), this );
	connect( rateDlg, SIGNAL(updateRate(int)), view, SLOT(setRate(int)) );
    }

    rateDlg->show();
}

