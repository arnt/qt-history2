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
#include <qcheckbox.h>

int main( int argc, char **argv )
{
	QApplication app( argc, argv );

	QTrayWidget widget;
	widget.setIcon( QPixmap( (const char**)edit_xpm ) );
	widget.setToolTip( "QTrayWidget" );

	QPopupMenu menu( &widget );
	menu.insertItem( "&Hide in System Tray" ); //, &cb, SLOT( animateClick() ) );
	menu.insertSeparator();
	menu.insertItem( "&Quit", &app, SLOT(quit()), Qt::CTRL+Qt::Key_Q );
	widget.setPopup( &menu );

	widget.show();

	QTrayWidget widget2;
	widget2.setIcon( QPixmap( (const char**)edit2_xpm ) );
	widget2.setToolTip( "QTrayWidget" );
	widget2.show();

	return app.exec();
}
