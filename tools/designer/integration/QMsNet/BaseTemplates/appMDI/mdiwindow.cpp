#include "mdiwindow.h"

#include <qapplication.h>
#include <qworkspace.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qtextedit.h>


MDIWindow::MDIWindow( QWidget* parent, const char* name, WFlags f )
    : MainWindow( parent, name, f )
{
    mdi = new QWorkspace( this, "MainWindow Workspace" );
    mdi->setBackgroundMode( PaletteDark );
    delete centralWidget(); // Remove UIC generated widget
    setCentralWidget( mdi );

    setCaption( tr("QMSNETPROJECTNAME") );
}


void MDIWindow::fileNew()
{
    // TODO: Use your own customized documentclass...
    QTextEdit *doc = new QTextEdit( mdi, "QMSNETPROJECTNAME document" );
    doc->setCaption( tr("Document%1").arg( mdi->windowList().count()+1 ) );
    doc->resize( 200, 100 );
    doc->show();
}
