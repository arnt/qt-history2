static const char * const maximize_xpm[]={
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
".##########.",
".##########.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".##########.",
"............",
"............"};


static const char * const minimize_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"...######...",
"...######...",
"............",
"............",
"............"};

static const char * const normalize_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"....######..",
"....######..",
"....#....#..",
"..######.#..",
"..######.#..",
"..#....###..",
"..#....#....",
"..#....#....",
"..######....",
"............",
"............"};

#include "trayicon.h"

#include <qapplication.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmainwindow.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QMainWindow mw;
    app.setMainWidget( &mw );

    QPopupMenu menu2;
    menu2.insertItem( "Test 2" );
    TrayIcon tray2( QPixmap( (const char**)minimize_xpm ), "Minimize MainWindow", &menu2 );
    QObject::connect(&tray2,SIGNAL(clicked(const QPoint&)),&mw,SLOT(showMinimized()));

    QPopupMenu menu;
    menu.insertItem( "Test 1" );
    menu.insertSeparator();
    menu.insertItem( "&Quit", &app, SLOT(quit()) );
    TrayIcon tray( QPixmap( (const char**)normalize_xpm ), "Show MainWindow", &menu );
    QObject::connect(&tray,SIGNAL(clicked(const QPoint&)),&mw,SLOT(showNormal()));

    TrayIcon tray3( QPixmap( (const char**)maximize_xpm ), "Maximize MainWindow" );
    QObject::connect(&tray3,SIGNAL(clicked(const QPoint&)),&mw,SLOT(showMaximized()));

    mw.show();
    tray.show();
    tray2.show();
    tray3.show();

    return app.exec();
}
