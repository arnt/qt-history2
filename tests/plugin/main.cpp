#include <qapplication.h>
#include <qmainwindow.h>
#include <qhbox.h>
#include <qscrollview.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtooltip.h>
#include <qaction.h>
#include <qlabel.h>

#include "qplugin.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    QWidgetFactory::installWidgetFactory( new QDefaultWidgetFactory );
    QPlugInManager* pm = new QPlugInManager("./plugin");
    QWidgetFactory::installWidgetFactory( pm );
    QActionFactory::installActionFactory( pm );

    QMainWindow mw;
    QScrollView sv( &mw );
    QHBox b( sv.viewport() );
    b.setFixedHeight( 200 );
    sv.addChild( &b );

    QStringList widgets = QWidgetFactory::widgetList();
    for ( uint i = 0; i < widgets.count(); i++ ) {
	QWidget* w = QWidgetFactory::createWidget( widgets[i], &b);
	if ( w )
	    QToolTip::add( w, QString(w->className()) + " (" + QWidgetFactory::widgetFactory( w->className() ) + ")" );
    }

    QPopupMenu pop;
    QStringList actions = QActionFactory::actionList();
    for ( uint j = 0; j < actions.count(); j++ ) {
	QAction* a = QActionFactory::createAction( actions[j], &mw );
	if ( a )
	    a->addTo( &pop );
    }
    
    mw.menuBar()->insertItem( "&AddIn", &pop );
    
    mw.setCentralWidget( &sv );

    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}
