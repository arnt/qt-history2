#include "sdiwindow.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qtextedit.h>


SDIWindow::SDIWindow( QWidget* parent, const char* name, WFlags f )
    : MainWindow( parent, name, f )
{
    doc = new QTextEdit( this, "MainWindow Document" );
    delete centralWidget(); // Remove UIC generated widget
    setCentralWidget( doc );

    setCaption( tr("QMSNETPROJECTNAME") );
}
