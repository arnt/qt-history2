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

#include <qtrayicon.h>

#include <qapplication.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmainwindow.h>

int main( int argc, char **argv )
{
	QApplication app( argc, argv );

	QMainWindow mw;
	app.setMainWidget( &mw );

	QPopupMenu menu;
	menu.insertItem( "Test 1" );
	menu.insertSeparator();
	menu.insertItem( "&Quit", &app, SLOT(quit()) );
	QTrayIcon tray( QPixmap( (const char**)edit_xpm ), "Hide MainWindow", &menu );

	QPopupMenu menu2;
	menu2.insertItem( "Test 2" );
	QTrayIcon tray2( QPixmap( (const char**)edit2_xpm ), "Show MainWindow", &menu );

	QObject::connect(&tray,SIGNAL(clicked(const QPoint&)),&mw,SLOT(hide()));
	QObject::connect(&tray2,SIGNAL(clicked(const QPoint&)),&mw,SLOT(show()));

	mw.show();
	tray.show();
	tray2.show();

	return app.exec();
}
