#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qaction.h>

#include "labelonoff.xpm"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QMainWindow * window = new QMainWindow;
    QToolBar * toolbar = new QToolBar( window );

    QAction * labelonoffaction = new QAction( "labels on/off", 
                                              "tool-button labels on/off", 
                                              Qt::ALT+Qt::Key_L,
                                              window, "labelonoff", TRUE );
    labelonoffaction->setIconSet( (QPixmap) labelonoff_xpm );                                 

    QObject::connect( labelonoffaction, SIGNAL( toggled( bool ) ), 
                      window, SLOT( setUsesTextLabel( bool ) ) );

    labelonoffaction->addTo( toolbar ); 

    app.setMainWidget( window );
    window->show();
    return app.exec();
}

