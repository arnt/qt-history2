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
    Main(QWidget* parent=0) :
	QWidget(parent),
	log(this)
    {
	log.setReadOnly(TRUE);
	log.setFont(QFont("courier",10));
	dump(0,(QKeyEvent*)0);
    }

    void resizeEvent(QResizeEvent*)
    {
	debug("Main got resized");
	log.setGeometry(0,0,width()/2,height());
    }

    void keyPressEvent(QKeyEvent* e)
    {
	dump("Kv", e);
    }

    void keyReleaseEvent(QKeyEvent* e)
    {
	dump("K^", e);
    }

    void mousePressEvent(QMouseEvent* e)
    {
	dump("Mv", e);
    }

    void mouseDoubleClickEvent(QMouseEvent* e)
    {
	dump("Mv2", e);
	if ( parentWidget() ) hide();
    }

    void mouseReleaseEvent(QMouseEvent* e)
    {
	dump("M^", e);
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

    void dump(const char* type, QMouseEvent* e)
    {
	QString line;
	line.sprintf("%2s %6s %3s %c  %3x", type, "", "",
		' ', e->state());
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
    Main mm(&m);
    mm.setGeometry(m.width()*3/4,0,80,80);
    a.setMainWidget( &m );
    m.show();
    return a.exec();
}
