//
// Qt Tutorial 4
//
//

#include <qapp.h>
#include <qpushbt.h>
#include <qfont.h>


class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
};

MyWidget::MyWidget( QWidget *parent=0, const char *name=0 )
        : QWidget( parent, name )
{
    QPushButton *quit = new QPushButton( "Quit!", this, "quit" );
    quit->move( 40, 40 );
    quit->resize( 120, 40 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( quit, SIGNAL(clicked()), qApp, SLOT(quitApp()) );
}


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MyWidget w;
    w.setGeometry( 100, 100, 200, 120 );
    w.setMinimumSize( 200, 120 );
    w.setMaximumSize( 200, 120 );

    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
