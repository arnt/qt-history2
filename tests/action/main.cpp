#include <qmainwindow.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qapplication.h>
#include <qpopupmenu.h>

#include <qmime.h>
#include <qpixmap.h>

#include "bold.xpm"
#include "italic.xpm"

#include "action.h"

#define Q_ICON( x ) QMimeSourceFactory::defaultFactory()->pixmap( x )

int main( int argc, char** argv )
{   
    QApplication app( argc, argv );

    QStringList lst;
    lst.append( "Helvetica" );
    lst.append( "Times" );
    lst.append( "Script" );
	    
    QMimeSourceFactory::defaultFactory()->setPixmap( "bold", QPixmap( bold ) );
    QMimeSourceFactory::defaultFactory()->setPixmap( "italic", QPixmap( italic ) );
	
    QActionCollection col;
    
    QAction* simple = new QAction( "Simple", Q_ICON( "bold" ), 0, &col );
    QAction* toggle = new QToggleAction( "Toggle", Q_ICON( "italic" ), 0, &col );
    QSelectAction* select = new QSelectAction( "Select", 0, &col );
    select->setItems( lst );
    QActionMenu* am = new QActionMenu( "Bookmarks", Q_ICON( "bold" ), &col );
    am->popupMenu()->insertItem( Q_ICON( "bold" ), "KDE Homepage" );
    am->popupMenu()->insertItem( Q_ICON( "italic" ), "KOffice Homepage"  );
    QAction* font = new QFontAction( "Font", Q_ICON( "italic" ), 0, &col );
    QAction* size = new QFontSizeAction( "Font size", Q_ICON( "bold" ), 0, &col );
    
    QMainWindow* m = new QMainWindow;
    QToolBar* bar = new QToolBar( m );
    QMenuBar* menu = m->menuBar();
    
    QPopupMenu* pop = new QPopupMenu( m );
    simple->plug( pop );
    toggle->plug( pop );
    select->plug( pop );
    am->plug( pop );
    font->plug( pop );
    size->plug( pop );
    menu->insertItem( "File", pop );
    
    simple->plug( bar );
    toggle->plug( bar );
    select->plug( bar );
    am->plug( bar );
    font->plug( bar );
    size->plug( bar );
    m->show();
    app.setMainWidget( m );
    
    app.exec();
}
