/*
$Id$
*/  

#include "editor.h"

#include <qtextedit.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qaction.h>

#include "red.xpm"
#include "black.xpm"


Editor::Editor() 
    : QMainWindow( 0, "main window")
{
    QActionGroup * colors = new QActionGroup( this, "colors", TRUE );

    QAction * setBlackFont = new QAction( "black", QPixmap( black_xpm ),
                                          "Font color: black", ALT+Key_B,
                                          colors, "blackfontcolor", TRUE );
    setRedFont = new QAction( "red", QPixmap( red_xpm ), "Font color: red",
                              ALT+Key_R, colors, "redfontcolor", TRUE );

    QObject::connect( colors, SIGNAL( selected( QAction * ) ),
                      this, SLOT( setFontColor( QAction * ) ) );

    QToolBar * toolbar = new QToolBar( this, "toolbar" );
    colors->addTo( toolbar );

    QPopupMenu * font = new QPopupMenu( this );
    menuBar()->insertItem( "&Font", font );

    colors->setUsesDropDown( TRUE );
    colors->setMenuText( "Font Color" );

    colors->addTo( font );

    editor = new QTextEdit( this, "editor" );
    setCentralWidget( editor );
}


void Editor::setFontColor( QAction * coloraction )
{
    if ( coloraction == setRedFont )
	editor->setColor( red );
    else
	editor->setColor( black );
}
