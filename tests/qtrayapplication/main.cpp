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

#include "qtrayapplication.h"
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>

int main( int argc, char **argv )
{
	QTrayApplication app( argc, argv );
	app.setTrayIcon( QPixmap( (const char**)edit_xpm ) );
	app.setToolTip( "QTrayApplication" );

	QCheckBox cb( "Show in System Tray", 0 );
	cb.show();
	app.setMainWidget( &cb );

	QPopupMenu menu;
	menu.insertItem( "&Hide in System Tray", &cb, SLOT( animateClick() ) );
	menu.insertSeparator();
	menu.insertItem( "&Quit", &app, SLOT(quit()), Qt::CTRL+Qt::Key_Q );
	app.setPopup( &menu );

	QObject::connect( &cb, SIGNAL(toggled(bool)), &app, SLOT(showInTray(bool)) );

	return app.exec();
}
