#include <qapplication.h>
#include <q3mainwindow.h>
#include <q3toolbar.h>
#include <qaction.h>

#include <qpixmap.h>

#include "labelonoff.xpm"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    Q3MainWindow * window = new Q3MainWindow;
    window->setWindowTitle("Qt Example - Toggleaction");
    Q3ToolBar * toolbar = new Q3ToolBar( window );

    QAction * labelonoffaction = new QAction( window, "labelonoff" );
    labelonoffaction->setToggleAction( TRUE );

    labelonoffaction->setText( "labels on/off" );
    labelonoffaction->setAccel( Qt::ALT+Qt::Key_L );
    labelonoffaction->setIconSet( (QPixmap) labelonoff_xpm );

    QObject::connect( labelonoffaction, SIGNAL( toggled( bool ) ),
                      window, SLOT( setUsesTextLabel( bool ) ) );

    labelonoffaction->addTo( toolbar );

    app.setMainWidget( window );
    window->show();
    return app.exec();
}

