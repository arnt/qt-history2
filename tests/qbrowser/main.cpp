#include <qapplication.h>
#include <qwindowsstyle.h>
#include <qml.h>
#include "lhelp.h"
#include <stdlib.h>

int main( int argc, char ** argv ) {
    QApplication::setColorSpec( QApplication::ManyColor );
    // don't let X change our color settings
    QApplication::useXResourceManager( FALSE );
    QApplication::setStyle( new QWindowsStyle );
    QApplication a(argc, argv);

    QMLStyle* style = new QMLStyle( QMLStyleSheet::defaultSheet(), "head" );
    style->setDisplayMode(QMLStyle::DisplayNone);
    QMLStyleSheet::defaultSheet()->style("p")->setSelfNesting( FALSE );


    QString qt = getenv("QTDIR");
    QString home;
    if (argc > 1)
	home = argv[1];
    else
	home = qt + "/doc/html/index.html";

    LHelp h(home, "");

    a.setMainWidget( &h );
    h.show();

    return a.exec();
}
