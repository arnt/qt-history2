#include "editor.h"

#include <qtextedit.h>
#include <qaction.h>   
#include <qpopupmenu.h>   
#include <qmenubar.h>   

#include "red.xpm"
#include "black.xpm"

Editor::Editor() : QMainWindow( 0, "main window")
{ 
    QActionGroup * colors = new QActionGroup( this, "colors", TRUE);
 
    QAction * setblackfont = new QAction( "black", QPixmap( black_xpm ), 
                                          "Font color: black", ALT+Key_B,
                                          colors, "blackfontcolor", TRUE );
    setredfont = new QAction( "red", QPixmap( red_xpm ), "Font color: red", 
                              ALT+Key_R, colors, "redfontcolor", TRUE );
 
    QObject::connect( colors, SIGNAL( selected( QAction * ) ), 
                      this, SLOT( setFontColor( QAction * ) ) );

    QToolBar * toolbar = new QToolBar( this, "toolbar" );
    colors->addTo( toolbar );     

    // members of colors now show up in a separate submenu 
    colors->setUsesDropDown( TRUE );
    // set a menu text for this submenu
    colors->setMenuText( "Font Color" );

    QPopupMenu * font = new QPopupMenu( this );
    menuBar()->insertItem( "&Font", font );    
    colors->addTo( font );

    editor = new QTextEdit( this, "editor" );
    setCentralWidget( editor );
}

Editor::~Editor()
{
}

void Editor::setFontColor( QAction * coloraction )
{
    if ( coloraction == setredfont )
	editor->setColor( red );
    else
	editor->setColor( black );
}
