#include <qapp.h>
#include <qscrbar.h>
#include <qlayout.h>
#include <qlcdnum.h>
#include <qheader.h>


//////////

#include "table.h"


class Urk : public QWidget
{
    Q_OBJECT
public:
    Urk( QWidget *parent = 0 );

protected:
    void resizeEvent( QResizeEvent * );

private:
    Table *table;
    QHeader *bar;
};

Urk::Urk( QWidget *parent )
    :QWidget( parent )
{
    bar = new QHeader( 6, this );

    bar->setGeometry( 0, 0, width(), bar->sizeHint().height() );
    bar->setLabel( 0, "Nothing" );
    bar->setLabel( 1, "One" );
    bar->setLabel( 2, "Second Opinion" );
    bar->setLabel( 3, "Number three, number three, number three" );

    table = new Table( bar, 10, this );
    table->setGeometry( 0, bar->height(), width(), height() - bar->height() );



}



void Urk::resizeEvent( QResizeEvent * )
{
    bar->resize( width(), bar->height() );
    table->resize( width(), width() - bar->height() );
}

#include "main.moc"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QWidget *w = new Urk;
    
    int x = 0;

    a.setMainWidget( w );
    w->show();
    return a.exec();
}
