static const char * const edit_xpm[]={
"16 16 5 1",
"b c #000000",
"# c #808080",
". c None",
"c c #ff0000",
"a c #ffffff",
"........####....",
"........#aa#b...",
"........#aa#ab..",
"........#aabbbb.",
"........#aaaccc.",
"........#aacccb.",
"........#acccab.",
"...ccc..#cccbbb.",
"....ccc.ccc.....",
".....ccccc......",
"......ccc.......",
"..b....c...b....",
".bb........bb...",
"bbbbbb..bbbbbb..",
".bb........bb...",
"..b........b...."};
static const char * const edit2_xpm[]={
"16 16 5 1",
"b c #000000",
"# c #808080",
". c None",
"c c #00ff00",
"a c #ffffff",
"........####....",
"........#aa#b...",
"........#aa#ab..",
"........#aabbbb.",
"........#aaaccc.",
"........#aacccb.",
"........#acccab.",
"...ccc..#cccbbb.",
"....ccc.ccc.....",
".....ccccc......",
"......ccc.......",
"..b....c...b....",
".bb........bb...",
"bbbbbb..bbbbbb..",
".bb........bb...",
"..b........b...."};

#include "qtraywidget.h"

#include <qapplication.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmainwindow.h>

int main( int argc, char **argv )
{
	QApplication app( argc, argv );

	QMainWindow mw;
	app.setMainWidget( &mw );

	QTrayWidget widget( &mw );
	widget.setIcon( QPixmap( (const char**)edit_xpm ) );
	widget.setToolTip( "QTrayWidget" );

	QPopupMenu menu;
	menu.insertItem( "&Hide in System Tray", &widget, SLOT( hide() ) );
	menu.insertSeparator();
	menu.insertItem( "&Quit", &app, SLOT(quit()), Qt::CTRL+Qt::Key_Q );
	widget.setPopup( &menu );

	QTrayWidget widget2( &mw );
	widget2.setIcon( QPixmap( (const char**)edit2_xpm ) );
	widget2.setToolTip( "QTrayWidget2" );

	QPopupMenu menu2( &widget );
	menu2.insertItem( "&Show the other guy", &widget, SLOT( show() ) );
	widget2.setPopup( &menu2 );

	widget.show();
	widget2.show();
	mw.show();

	return app.exec();
}
