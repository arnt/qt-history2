#include <qwellarray.h>

#include <qpushbt.h>
//#include <qlined.h>
#include <qapp.h>
#include <qlayout.h>
#include <qdrawutl.h>
#include <qlabel.h>
//#include <qevent.h>
//#include "qobjcoll.h"

class Pop : public QWellArray
{
    Q_OBJECT
public:
    Pop( QWidget *p, const char *n ) : pp(p), QWellArray(0,n,TRUE) {}
public slots:
    void popup();

protected:
    void mouseReleaseEvent( QMouseEvent * );

private:
    QWidget *pp;
};

void Pop::popup()
{
    move ( pp->mapToGlobal(QPoint(0, pp->height()+1) ) );
    setCurrent( 0,0 );
    setSelected( 0,0 );
    show();
}
void Pop::mouseReleaseEvent( QMouseEvent * )
{
    hide();
}


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QWidget m;

    QWellArray w( &m );

    int nRows = 7;
    int nCols = 13;
    int row,col;
    w.setDimension( nRows, nCols );
    for ( row = 0; row < nRows; row++ )
	for ( col = 0; col < nCols; col++ ) {
	    int red = (255 * row) / nRows;
	    int blue = 255 - red;
	    int green = (255 * col) / nCols;

	    QColor c( red, green, blue );
	    w.setCellBrush( row, col, c );
	}



    //    QPalette p( green );
    //    w.setPalette( p );
    /*  
	if ( w.style() == WindowsStyle )
	w.setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	else
	w.setFrameStyle( QFrame::Panel | QFrame::Raised );
	*/

    w.move( 10, 10 );
    w.resize( w.sizeHint() );

    QPushButton b( "Press Me!", &m );
    b.resize( b.sizeHint() );
    b.move ( 10, w.geometry().bottom()+10 );


    Pop p( &b, "Well" );
    
    nRows = 7;
    nCols = 5;
    p.setDimension( nRows, nCols );
    for ( row = 0; row < nRows; row++ )
	for ( col = 0; col < nCols; col++ ) {
	    int red = (255 * row) / nRows;
	    //	    int blue = 255 - red;
	    int green = 55+ (200 * col) / nCols;

	    QColor c( red, green, 0 );
	    p.setCellBrush( row, col, c );
	}
    p.resize( p.sizeHint() );
    //    p.move ( m.mapToGlobal(QPoint(10, b.geometry().bottom()+1) ) );
    //    p.raise();

    QObject::connect( &b, SIGNAL( clicked() ), &p, SLOT( popup() ) );


    a.setMainWidget( &m );
    m.show();


    return a.exec();
}
#include "well.moc"
