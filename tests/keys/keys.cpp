//
// Qt Example Application: keys
//
// Demonstrates Qt key events.
//

#include <qapp.h>
#include <qwidget.h>
#include <qmlined.h>

class Main : public QWidget {
public:
    Main() :
	log(this)
    {
	log.setReadOnly(TRUE);
	log.setFont(QFont("courier",10));
	dump(0,0);
    }

    void resizeEvent(QResizeEvent*)
    {
	log.setGeometry(0,0,width()/2,height());
    }

    void keyPressEvent(QKeyEvent* e)
    {
	dump("Pr", e);
    }

    void keyReleaseEvent(QKeyEvent* e)
    {
	dump("Re", e);
    }

    void dump(const char* type, QKeyEvent* e)
    {
	QString line;
	if (!type) 
	    line.sprintf("%2s %6s %3s    %3s", "", "key", "asc", "sta");
	else
	    line.sprintf("%2s %6x %3x(%c) %3x", type, e->key(), e->ascii(),
		e->ascii() ? e->ascii() : ' ', e->state());
	log.insertLine( line );
	log.setCursorPosition(999999,0);
    }

private:
    QMultiLineEdit log;
};

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a( argc, argv );
    QApplication::setFont( QFont("Helvetica") );

    Main m;
    a.setMainWidget( &m );
    m.show();
    return a.exec();
}
