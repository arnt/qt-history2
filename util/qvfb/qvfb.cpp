
#include "qvfb.h"
#include "qvfbview.h"

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>

QVFb::QVFb( int w, int h, int d, QWidget *parent, const char *name,
			uint flags )
    : QMainWindow( parent, name, flags )
{
    QString cap("Virtual framebuffer %1x%2 %3bpp");
    setCaption( cap.arg(w).arg(h).arg(d) );
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

    menuBar()->insertItem( "&View", viewMenu );
}

void QVFb::slotCursor()
{
    enableCursor( !viewMenu->isItemChecked( cursorId ) );
}

