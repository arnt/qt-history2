//
// Qt Tutorial 6
//
//

#include <qapp.h>
#include <qpushbt.h>
#include <qscrbar.h>
#include <qlcdnum.h>
#include <qfont.h>


class LCDRange : public QWidget
{
public:
    LCDRange( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QScrollBar  *sBar;
    QLCDNumber  *lcd;
};

LCDRange::LCDRange( QWidget *parent=0, const char *name=0 )
        : QWidget( parent, name )
{
    lcd  = new QLCDNumber( 2, this, "lcd"  );
    lcd->move( 0, 0 );
    sBar = new QScrollBar( 0, 99, 1, 10, 0, QScrollBar::Horizontal, 
                           this, "scrollbar" );
    connect( sBar, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)) );
}

void LCDRange::resizeEvent( QResizeEvent *e )
{
    lcd->resize( width(), height() - 16 - 5 );
    sBar->setGeometry( 0, lcd->height() + 5, width(), 16 );
}


class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QPushButton *quit;
    LCDRange *value[16];
};


MyWidget::MyWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 200, 300 );

    quit = new QPushButton( "Quit", this, "quit" );
    quit->setGeometry( 10, 10, 75, 30 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quitApp()) );

    for( int i = 0 ; i < 16 ; i++ )
	value[i] = new LCDRange( this );
}

void MyWidget::resizeEvent( QResizeEvent *e )
{
    int valueWidth = (width() - 20)/4;
    int valueHeight = (height() - 65)/4;
    for( int i = 0 ; i < 16 ; i++ )
	value[i]->setGeometry( 10 + (i%4)*valueWidth,  55 + (i/4)*valueHeight,
                               valueWidth - 5, valueHeight - 5 );
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
