#include <qapplication.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include <stdlib.h>

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );

    QWidget * t = new QWidget;

    int w = QApplication::desktop()->width()-100;
    int h = QApplication::desktop()->height()-100;

    QGridLayout * g = new QGridLayout( t, 1, 1, 4, 4 );
    int x = 0;
    int column = 0;
    int n = 0;
    while( x < w ) {
	QWidget * w;
	int y=0;
	int row = 0;
	int c = 0;
	while( y < h ) {
	    switch( rand()%3 ) {
	    case 0:
		w = new QSpinBox( n, n+100, 1, t );
		n++;
		break;
	    case 1:
		w = new QComboBox( TRUE, t );
		for( c = n; c < n+10; c++ )
		    ((QComboBox *)w)->insertItem( QString::number( c ) );
		n++;
		break;
	    case 2:
		w = new QPushButton( QString::number( n++ ), t );
		break;
	    }
	    g->addWidget( w, row, column );
	    row++;
	    y += 40;
	}
	column++;
	x += 100;
    }

    QObject::connect( qApp, SIGNAL(lastWindowClosed()),
		      qApp, SLOT(quit()) );

    t->resize( t->sizeHint() );
    t->show();
    return a.exec();
}
