#include <qapplication.h>
#include <qscrollbar.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qheader.h>


//////////

//#include "table.h"
#include <limits.h>

class Urk : public QWidget
{
    Q_OBJECT
public:
    Urk( QWidget *parent = 0 );

protected:
    void resizeEvent( QResizeEvent * );

private:
    //Table *table;
    QHeader *bar;
    QHeader *vbar;
    QScrollBar *scr;
    enum {  HeadCount = 3000000, HeadSize = INT_MAX/HeadCount };
};



Urk::Urk( QWidget *parent )
    :QWidget( parent )
{
    bar = new QHeader( HeadCount, this );
    //bar = new QHeader( 4, this );
    bar->setGeometry( 0, 0, width(), bar->sizeHint().height() );
    bar->setLabel( 0, "Nothing" );
    bar->setLabel( 1, "Noresize, noclick" );
    bar->setLabel( 2, "Second Opinion" );
    bar->setLabel( 3, "Noresize, number three, number three, number three" );

    for ( int i = 0; i < HeadCount; i++ )
	bar->setCellSize( i, HeadSize );
    

    //    table = new Table( bar, 10, this );
    //    table->setGeometry( 0, bar->height(), width(), height() - bar->height() );


    bar->setResizeEnabled( FALSE, 1 );
    bar->setResizeEnabled( FALSE, 3 );
    bar->setClickEnabled( FALSE, 1 );


    scr = new QScrollBar( QScrollBar::Horizontal, this );
    scr->setGeometry( 0, bar->height()+2, width(), scr->sizeHint().height() );

    scr->setRange( 0, QMAX(0,HeadCount*HeadSize - bar->width()) );
    scr->setSteps( 80, bar->width() - 40 ); 

    connect( scr, SIGNAL(valueChanged(int)), bar, SLOT(setOffset(int)) );

#if 0
    vbar = new QHeader( 10, this );
    vbar->setOrientation( Vertical );
    vbar->setResizeEnabled( FALSE, 1 );
    vbar->setResizeEnabled( FALSE, 3 );
    vbar->setClickEnabled( FALSE, 1 );

    vbar->setGeometry( 0, bar->height(), vbar->sizeHint().width(), height() - bar->height() );
    vbar->setLabel( 0, "Nothing" );
    vbar->setLabel( 1, "Noresize, noclick" );
    vbar->setLabel( 2, "Second Opinion" );
    vbar->setLabel( 3, "Noresize, number three, number three, number three" );
#endif

}



void Urk::resizeEvent( QResizeEvent * )
{
    bar->resize( width(), bar->height() );
    scr->setGeometry( 0, bar->height()+2, width(), scr->sizeHint().height() );

    scr->setRange( 0, QMAX(0,HeadCount*HeadSize - bar->width()) );
    scr->setSteps( 80, bar->width() - 40 ); 


    //vbar->setGeometry( 0, bar->height(), vbar->sizeHint().width(), height() - bar->height() );
    //     table->resize( width(), height() - bar->height() );
}

#include "main.moc"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QWidget *w = new Urk;

    a.setMainWidget( w );
    w->show();
    return a.exec();
}
