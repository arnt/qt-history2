#include "plugmainwindow.h"
#include "qwidgetfactory.h"
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qfiledialog.h>

PlugMainWindow::PlugMainWindow( QWidget* parent, const char* name, WFlags f )
: QMainWindow( parent, name, f )
{
    QPopupMenu* file = (QPopupMenu*)QWidgetFactory::create( "QPopupMenu", this );
    if ( !file )
	return;

    file->insertItem( "&Open", this, SLOT(fileOpen()) );
    menuBar()->insertItem( "&File", file );
    statusBar();
}

void PlugMainWindow::fileOpen()
{
/*    QString file = QFileDialog::getOpenFileName( QString::null, QWidgetFactory::fileTypeList().join(";;"), this, 0, "File Open" );

    if ( file.isEmpty() )
	return;

    QWidgetFactory::createWidget( file );
*/    
}
