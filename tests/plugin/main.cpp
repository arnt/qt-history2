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

    // installing all available factories
    QWidgetFactory::installWidgetFactory( new QDefaultWidgetFactory ); 
    QPlugInManager* pm = new QPlugInManager("./plugin");
    QWidgetFactory::installWidgetFactory( pm );
    QActionFactory::installActionFactory( pm );

    bool ok;
    QWidget* loaded = QWidgetFactory::createWidget( "widgetpreview.ui", ok );
    if ( loaded )
	loaded->show();

    // little test scenario
    QMainWindow mw;
    QScrollView sv( &mw );
    QHBox b( sv.viewport() );
    b.setFixedHeight( 200 );
    sv.addChild( &b );

    // creating all available widgets and adding them to the little test scenario
    QStringList widgets = QWidgetFactory::widgetList();
    for ( uint i = 0; i < widgets.count(); i++ ) {
	QWidget* w = QWidgetFactory::createWidget( widgets[i], &b);
	if ( w )
	    QToolTip::add( w, QString(w->className()) + " (" + QWidgetFactory::widgetFactory( w->className() ) + ")" );
    }

    // creating a nice popupmenu and add all available actions
    QPopupMenu pop;
    QStringList actions = QActionFactory::actionList();
    for ( uint j = 0; j < actions.count(); j++ ) {
	QAction* a = QActionFactory::createAction( actions[j], &mw );
	if ( a )
	    a->addTo( &pop );
    }

    mw.menuBar()->insertItem( "&AddIn", &pop );
    
    // the rest ist silence...
    mw.setCentralWidget( &sv );
    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}
