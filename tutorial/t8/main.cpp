//
// Qt Tutorial 8
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
    LCDRange    *angle;
    CannonField *cannon;
};


MyWidget::MyWidget( QWidget *parent=0, const char *name=0 )
        : QWidget( parent, name )
{
    quit = new QPushButton( "Quit", this, "quit" );
    quit->setGeometry( 10, 10, 75, 30 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quitApp()) );

    angle  = new LCDRange( this, "angle" );
    angle->setRange( 5, 85 );
    angle->move( 10, 45 );
    cannon = new CannonField( this, "canonfield" );
    cannon->resize( 400, 300 );
    cannon->setBackgroundColor( QColor( 250, 250, 200) );
    connect( angle, SIGNAL(valueChanged(int)), cannon, SLOT(setAngle(int)) );
    angle->setValue( 45 );
}

void MyWidget::resizeEvent( QResizeEvent *e )
{
    angle->resize( width() - 425, 100 );
    cannon->move( angle->x() + angle->width() + 5, 45 );
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    MyWidget w;
    w.setGeometry( 100, 100, 500, 355 );
    w.setMinimumSize( 500, 355 );

    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
