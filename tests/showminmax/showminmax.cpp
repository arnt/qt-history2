#include <qapplication.h>
#include <qlabel.h>
#include <stdlib.h>


class Main : public QWidget
{
public:
    Main()
    {
	setCaption( "IDLE" );
    }

    void timerEvent( QTimerEvent * )
    {
	doIt( rand() % 5 );
    }

    void keyPressEvent( QKeyEvent *e )
    {
	if ( e->ascii() >= '0' && e->ascii() <= '4' )
	    doIt( e->ascii() - '0' );
	else if ( tolower(e->ascii()) == 't' )
	    startTimer( 1000 );
    }

    void doIt( int n )
    {
	switch ( n ) {
	    case 0:
		setCaption("HIDE");
		hide();
		break;
	    case 1:
		setCaption("SHOW");
		show();
		break;
	    case 2:
		setCaption("MAX");
		showMaximized();
		break;
	    case 3:
		setCaption("MIN");
		showMinimized();
		break;
	    case 4:
		setCaption("NORMAL");
		showNormal();
		break;
	}
    }	
};


int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    Main m;
    a.setMainWidget(&m);
    m.show();

    return a.exec();
}
