#include <qapplication.h>
#include <qwindowsstyle.h>
#include <qstylesheet.h>
#include "lhelp.h"
#include <stdlib.h>


int main( int argc, char ** argv ) {
    QApplication::setColorSpec( QApplication::ManyColor );
    // don't let X change our color settings
//     QApplication::useXResourceManager( FALSE );

    QApplication::setDesktopSettingsAware( FALSE );
    QApplication::setStyle( new QWindowsStyle );
    QApplication a(argc, argv);

    QStyleSheetItem* style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "head" );
    style->setDisplayMode(QStyleSheetItem::DisplayNone);
    QStyleSheet::defaultSheet()->item("p")->setSelfNesting( FALSE );

    
    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "dl" );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);

    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "dt" );
    style->setDisplayMode(QStyleSheetItem::DisplayListItem);
    style->setContexts("dl");

    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "dd" );

    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "big" );
    style->setFontSize( 24 );
    
    QString qt = getenv("QTDIR");
    QString home;

    bool slideshow = FALSE;
    if (argc > 2  && QString("-slideshow") == argv[1] ) {
	slideshow = TRUE;
    }
    else if (argc > 1)
	home = argv[1];
    else
	home = qt + "/doc/html/index.html";

    LHelp h(home, "");

    if (slideshow) {
	h.setupSlideshow( argv[2] );
    }

    a.setMainWidget( &h );
    h.show();

    return a.exec();
}
