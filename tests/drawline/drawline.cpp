#include <qapplication.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qwmatrix.h>

class cDrawWindow : public QWidget {
private:
  void Draw(QPainter *p, int n, bool c);
public:
  cDrawWindow(QWidget *parent) : QWidget(parent) { setBackgroundMode( PaletteLight ); }
  virtual void paintEvent(QPaintEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  };

bool d = FALSE;

void cDrawWindow::Draw(QPainter *p, int n, bool c)
{
  int x1 = width() / 10 + n;
  int y1 = height() / 10 + n;
  int x2 = width() - x1;
  int y2 = height() - y1;
  p->drawLine(x1, y1, x2, y1); // upper
  p->drawLine(x1, y2, x2, y2); // lower
  QPen bluePen = p->pen();
  bluePen.setColor( "blue" );
  p->setPen( bluePen );
  p->drawLine(x1, y1, x1, y2); // left
  p->drawLine(x2, y1, x2, y2); // right
  //qDebug( "start = %i, %i, stop = %i, %i", x1, y1, x2, y2 );
  if ( d ) {
      QPen pen = p->pen();
      p->setPen( QPen("red", 0 ) );
      p->drawPoint( x1, y1 );
      p->drawPoint( x2, y2 );
      p->drawPoint( x2, y1 );
      p->drawPoint( x1, y2 );
      p->setPen( pen );
  }
  
  if ( !n )
      return;

  if ( c ) {
      x1 += 10;
      y1 += 10;
      x2 -= 20;
      y2 -= 20;
      p->drawLine( x1, y1, x2, y2 );
      if ( d ) {
	  QPen pen = p->pen();
	  p->setPen( QPen("red", 0 ) );
	  p->drawPoint( x1, y1 );
	  p->drawPoint( x2, y2 );
	  p->setPen( pen );
      }
            
      QPointArray a1( 3 );
      x1 += 30;
      a1.setPoint( 0, x1, y1 );
      a1.setPoint( 1, (x1+x2)/2, (y1+y2)/2 );
      a1.setPoint( 2, x2, y1 );
      p->drawPolyline( a1 );
      if ( d ) {
	  QPen pen = p->pen();
	  p->setPen( QPen("red", 0 ) );
	  p->drawPoint( x1, y1 );
	  p->drawPoint( x2, y1 );
	  p->setPen( pen );
      }

      x1 -= 30;
      y1 += 30;
      x2 -= 30;

      QPointArray a2( 3 );
      a2.setPoint( 0, x1, y1 );
      a2.setPoint( 1, x1, y2 );
      a2.setPoint( 2, x2, y2 );
      p->drawPolyline( a2 );

      if ( d ) {
	  QPen pen = p->pen();
	  p->setPen( QPen("red", 0 ) );
	  p->drawPoint( x1, y1 );
	  p->drawPoint( x2, y2 );
	  p->setPen( pen );
      }
      
      x1 += 10;
      y2 -= 10;
      QPointArray a3( 2 );
      a3.setPoint( 0, x1, y1 );
      a3.setPoint( 1, x1, y2 );
      p->drawLineSegments( a3 );


      if ( d ) {
	  QPen pen = p->pen();
	  p->setPen( QPen("red", 0 ) );
	  p->drawPoint( x1, y1 );
	  p->drawPoint( x1, y2 );
	  p->setPen( pen );
      }

      x1 += 10;
      x2 -= 10;

      QPointArray a4( 2 );
      a4.setPoint( 0, x1, y1 );
      a4.setPoint( 1, x2, y2 );
      p->drawLineSegments( a4 );


      if ( d ) {
	  QPen pen = p->pen();
	  p->setPen( QPen("red", 0 ) );
	  p->drawPoint( x1, y1 );
	  p->drawPoint( x2, y2 );
	  p->setPen( pen );
      }
      
  }	
}

void cDrawWindow::mousePressEvent(QMouseEvent *)
{
    d = !d;
    update();
}

int w = 0;
Qt::PenStyle line = Qt::SolidLine;
Qt::PenCapStyle cap = Qt::FlatCap;
Qt::PenJoinStyle join = Qt::MiterJoin;

void cDrawWindow::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  p.setPen(QPen("black", 0, line ) );
  Draw(&p, 0, FALSE);
  p.setPen(QPen("black", w, line , cap, join) );
  Draw(&p, 10, TRUE);
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  if ( argc > 1 ) {
      QString ws = argv[1];
      w = ws.toInt();
  }
  if ( argc > 2 ) {
      QString ws = argv[2];
      if ( ws[0] == 'r' ) {
	  cap = Qt::RoundCap;
	  qDebug( "Cap: Round" );
      }
      else if ( ws[0] == 's' ) {
	  cap = Qt::SquareCap;
	  qDebug( "Cap: Square" );
      }
      else {
	  qDebug( "Cap: Flat" );
      }
  }
  if ( argc > 3 ) {
      QString ws = argv[3];
      if ( ws[0] == 'r' ) {
	  join = Qt::RoundJoin;
	  qDebug( "Join: Round" );
      }
      else if ( ws[0] == 'b' ) {
	  join = Qt::BevelJoin;
	  qDebug( "Join: Bevel" );
      }
      else {
	  qDebug( "Join: Miter" );
      }
  }

  if ( argc > 4 )
      line = Qt::DashLine;

  QMainWindow mw;
  a.setMainWidget(&mw);
  cDrawWindow w(&mw);
  mw.setCentralWidget(&w);
  mw.resize( 300, 200 );

  mw.show();
  a.exec();
  return 0;
}
