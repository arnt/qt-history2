//
// Qt Tutorial 5
//
//

#include <qapp.h>
#include <qpushbt.h>
#include <qscrbar.h>
#include <qlcdnum.h>
#include <qfont.h>

class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QPushButton *quit;
    QScrollBar  *sBar;
    QLCDNumber  *lcd;
};


MyWidget::MyWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 200, 200 );

    quit = new QPushButton( "Quit", this, "quit" );
    quit->setGeometry( 10, 10, 75, 30 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quitApp()) );

    lcd  = new QLCDNumber( 2, this, "lcd" );
    lcd->move( 10, 50 );
    sBar = new QScrollBar( 0, 99, 1, 10, 0, QScrollBar::Horizontal, 
                           this, "scrollbar" );
    connect( sBar, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)) );
}

void MyWidget::resizeEvent( QResizeEvent *e )
{
    lcd->resize( width() - 20, height() - 81 );
    sBar->setGeometry( 10, lcd->y() + lcd->height() + 5, width() - 20, 16 );
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
