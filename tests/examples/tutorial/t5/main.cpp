/****************************************************************
**
** Qt tutorial 5
**
****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qlcdnumber.h>
#include <qfont.h>

#include <qlayout.h>

class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
};


MyWidget::MyWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    QPushButton *quit = new QPushButton( "Quit", this, "quit" );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    QLCDNumber *lcd  = new QLCDNumber( 2, this, "lcd" );

    QScrollBar *sBar = new QScrollBar( 0, 99,		       	// range
			   1, 10, 			// line/page steps
			   0, 				// inital value
			   QScrollBar::Horizontal, 	// orientation
                           this, "scrollbar" );

    connect( sBar, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)) );

    QVBoxLayout *vbox = new QVBoxLayout( this, 5 );

    vbox->addWidget( quit, 0, AlignLeft );
    vbox->addWidget( lcd );  //stretch unnecessary, since lcd wants to grow
    vbox->addWidget( sBar );
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MyWidget w;
    w.setGeometry( 100, 100, 200, 200 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
