#include <qpicture.h>
#include <qapplication.h>
#include <qpainter.h>

class DisplayWidget : public QWidget {
public:
    DisplayWidget( const QPicture &p ) : pic( p ), sc( 1.0 ) { }
    void paintEvent( QPaintEvent * );
    void keyPressEvent( QKeyEvent *k );

private:
    QPicture pic;
    double sc;
};

void DisplayWidget::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    p.scale( sc, sc );
    p.drawPicture( pic );
    p.end();
}

void DisplayWidget::keyPressEvent( QKeyEvent *k )
{
    switch ( k->ascii() ) {
    case '+':
	sc++;
	update();
	break;
    case '-':
	sc--;
	update();
	break;
    }
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    if ( a.argc() > 1 ) {
        QPicture pic;
        if ( !pic.load( a.argv()[1], "svg" ) ) {
            qWarning( "Loading of %s failed.", a.argv()[1] );
            return 1;
        }
        DisplayWidget w( pic );
        a.setMainWidget( &w );
        w.show();
        return a.exec();
    }

    qWarning( "Usage: %s {file}", argv[0] );
    return 1;
}
