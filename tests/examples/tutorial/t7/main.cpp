/****************************************************************
**
** Qt tutorial 7
**
****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qlcdnumber.h>
#include <qfont.h>
#include <qlayout.h>

#include "lcdrange.h"


class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
};


MyWidget::MyWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 200, 300 );

    QVBoxLayout *vbox = new QVBoxLayout( this, 10 );
    
    QPushButton *quit = new QPushButton( "Quit", this, "quit" );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    vbox->addWidget( quit, 0, Qt::AlignRight );
    
    QGridLayout *grid = new QGridLayout( 4, 4, 1 ); //#### 4 x 4 with 1 spacing
    vbox->addLayout( grid );
    
    LCDRange *previous = 0;
    for( int c = 0 ; c < 4 ; c++ )
	for( int r = 0 ; r < 4 ; r++ ) {
	    LCDRange* lr = new LCDRange( this );
	    grid->addWidget( lr, r, c );
	    if ( previous )
		connect( lr , SIGNAL(valueChanged(int)),
			 previous , SLOT(setValue(int)) );
	    previous = lr;
	}
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MyWidget w;
    w.setGeometry( 100, 100, 400, 400 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
