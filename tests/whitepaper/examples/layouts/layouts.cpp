/*
  layouts.cpp
*/

#include <qapplication.h>
#include <qgrid.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qvbox.h>

void create_label( const QString& text, QWidget *parent )
{
    QLabel *label = new QLabel( "<font color=blue><b>" + text, parent );
    label->setAlignment( Qt::AlignCenter );
    label->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    label->setLineWidth( 2 );
    label->setMidLineWidth( 1 );
    label->setFixedSize( 70, 20 );
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QHBox *hbox = new QHBox;
    hbox->setMargin( 11 );
    hbox->setSpacing( 6 );
    create_label( "0", hbox );
    create_label( "1", hbox );
    create_label( "2", hbox );
    create_label( "3", hbox );
    create_label( "4", hbox );
    hbox->show();

    QVBox *vbox = new QVBox;
    vbox->setMargin( 11 );
    vbox->setSpacing( 6 );
    create_label( "0", vbox );
    create_label( "1", vbox );
    create_label( "2", vbox );
    create_label( "3", vbox );
    create_label( "4", vbox );
    vbox->show();

    QGrid *grid = new QGrid( 2 );
    grid->setMargin( 11 );
    grid->setSpacing( 6 );
    create_label( "0, 0", grid );
    create_label( "0, 1", grid );
    create_label( "1, 0", grid );
    create_label( "1, 1", grid );
    create_label( "2, 0", grid );
    grid->show();

    QObject::connect( &app, SIGNAL(lastWindowClosed()),
		      &app, SLOT(quit()) );
    return app.exec();
}
