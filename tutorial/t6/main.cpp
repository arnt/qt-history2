/****************************************************************
**
** Qt tutorial 6
**
****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qlcdnumber.h>
#include <qfont.h>
#include <qlayout.h>

class LCDRange : public QWidget
{
public:
    LCDRange( QWidget *parent=0, const char *name=0 );
};

LCDRange::LCDRange( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    QLCDNumber *lcd  = new QLCDNumber( 2, this, "lcd"  );
    QScrollBar* sBar = new QScrollBar( 0, 99, 	// range
				       1, 10,	// line/page steps
				       0,	// inital value
				       QScrollBar::Horizontal, 	// orientation
				       this, "scrollbar" );
    connect( sBar, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)) );
    QVBoxLayout *vbox = new QVBoxLayout( this, 5 );
    vbox->addWidget( lcd );
    vbox->addWidget( sBar );
}

class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
};


MyWidget::MyWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 200, 300 );

    QVBoxLayout *vbox = new QVBoxLayout( this );

    QPushButton *quit = new QPushButton( "Quit", this, "quit" );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    vbox->addWidget( quit, 0, Qt::AlignRight );

    QGridLayout *grid = new QGridLayout( 4, 4 );
    vbox->addLayout( grid );

    for( int c = 0 ; c < 4 ; c++ ) {
	for( int r = 0 ; r < 4 ; r++ ) {
	    LCDRange* lr = new LCDRange( this );
	    grid->addWidget( lr, r, c );
	}
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
