//
// Qt Tutorial 12
//
//

#include <qapp.h>
#include <qpushbt.h>
#include <qscrbar.h>
#include <qlcdnum.h>
#include <qfont.h>

#include "lcdrange.h"
#include "cannon.h"


class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QPushButton *quit;
    QPushButton *shoot;
    LCDRange    *angle;
    LCDRange    *force;
    CannonField *cannon;
};


MyWidget::MyWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 500, 355 );

    quit = new QPushButton( "Quit", this, "quit" );
    quit->setGeometry( 10, 10, 75, 30 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quitApp()) );

    angle  = new LCDRange( "ANGLE", this, "angle" );
    angle->setRange( 5, 70 );
    angle->move( 10, 45 );

    force  = new LCDRange( "FORCE", this, "force" );
    force->setRange( 10, 50 );
    force->move( 10, 180 );

    cannon = new CannonField( this, "canonfield" );
    cannon->resize( 400, 300 );
    cannon->setBackgroundColor( QColor( 250, 250, 200) );

    connect( angle, SIGNAL(valueChanged(int)), cannon, SLOT(setAngle(int)) );
    connect( force, SIGNAL(valueChanged(int)), cannon, SLOT(setForce(int)) );

    angle->setValue( 45 );
    force->setValue( 25 );

    shoot = new QPushButton( "Shoot", this, "shoot" );
    shoot->setGeometry( 90, 10, 75, 30 );
    shoot->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( shoot, SIGNAL(clicked()), cannon, SLOT(shoot()) );

}

void MyWidget::resizeEvent( QResizeEvent *e )
{
    angle->resize( width() - 425, 130 );
    force->resize( width() - 425, 130 );
    cannon->move( angle->x() + angle->width() + 5, 45 );
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MyWidget w;
    w.setGeometry( 100, 100, 500, 355 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
