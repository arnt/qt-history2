#include "qimagepaintdevice.h"
#include <qapplication.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <stdlib.h>

class Main : public QWidget {
public:
    Main() :
	qipd(640,480)
    {
	resize(qipd.image().size());
	painter.begin(&qipd);
	clip = rect();
	mode = 'I';
	painter.setClipRegion(clip);
    }

    ~Main()
    {
	painter.end();
    }

    void keyPressEvent(QKeyEvent* e)
    {
	char shape=0;

	switch ( e->ascii() ) {
	  case 'c':
	    for ( int i=0; i<100; i++ ) {
		QRect rnd(rand()%width(),rand()%height(),rand()%50,rand()%50);
		clip -= rnd;
	    }
	    painter.setClipRegion(clip);
	    break;
	  case 'l':
	  case 'r':
	  case 'p':
	  case 't':
	    shape = e->ascii();
	    break;
	  case 'S':
	  case 'I':
	  case 'P':
	    mode = e->ascii();
	    break;
	  case '1':
	    font = QFont("Helvetica",12);
	    break;
	  case '2':
	    font = QFont("babelfish",12);
	    break;
	  case '3':
	    font = QFont("babelfish",36);
	    break;
	}

	if ( mode && shape ) {
	    QPainter *ptr=0;

	    QPixmap pm(width(),height());
	    QPainter ppm(&pm);
	    if ( mode == 'P' ) {
		ppm.setBrush(cyan);
		ppm.setClipRegion(clip);
		ptr = &ppm;
	    }

	    QPainter px11(this);
	    if ( mode == 'S' ) {
		px11.setBrush(yellow);
		px11.setClipRegion(clip);
		ptr = &px11;
	    }

	    painter.setPen(magenta);
	    if ( mode == 'I' ) {
		ptr = &painter;
	    }

	    if ( shape=='l' || shape=='t' ) {
		painter.setPen(magenta);
		px11.setPen(yellow);
		ppm.setPen(cyan);
	    } else {
		painter.setPen(white);
		px11.setPen(NoPen);
		ppm.setPen(NoPen);
		px11.setBrush(blue);
	    }

	    QPointArray poly(50);
	    if ( shape == 'p' )
		for (int p=0; p<50; p++) {
		    poly[p] = QPoint(rand()%width()/2,rand()%height()/2);
		}
	    else if ( shape == 't' )
		ptr->setFont(font);

	    QTime t;
	    t.start();
	    int x=0,y=0;
	    int w=width(),h=height();
	    int n=shape == 'l' ? 5000 : 500;
	    int pix=0;
	    for (int i=0; i<n; i++) {
		if ( shape=='r' ) {
		    int rw = (12345*i) % 100 + 20;
		    int rh = (12345+i) % 100 + 20;
		    ptr->drawRect(x,y,rw,rh);
		    x += 12345*i;
		    y += 2345+i;
		    x %= w;
		    y %= h;
		    pix+=rw*rh;
		} else if ( shape=='p' ) {
		    poly.translate(x,y);
		    ptr->drawPolygon(poly);
		    poly.translate(-x,-y);
		    x += 12345*i;
		    y += 2345+i;
		    x %= w;
		    y %= h;
		    pix += 10000; // bogus
		} else if ( shape=='l' ) {
		    ptr->lineTo(x,y);
		    int ox=x,oy=y;
		    x += 12345*i;
		    y += 2345+i;
		    x %= w;
		    y %= h;
		    pix+=QMAX(QABS(ox-x),QABS(oy-y));
		} else if ( shape=='t' ) {
		    ptr->drawText(x,y,"Hello World");
		    x += 12345*i;
		    y += 2345+i;
		    x %= w;
		    y %= h;
		    pix+=500; // bogus
		}
	    }
	    qApp->syncX();
	    int el = t.elapsed();
	    qDebug("%s%c: %dms for %d %c's (%d pixels/second)",
#ifdef DEBUG
		"Unoptimized: ",
#else
		"",
#endif
		mode, el,n,shape, el ? pix/el*1000 : 9999999);
	    repaint(FALSE);
	}
    }

    void mousePressEvent(QMouseEvent* e)
    {
	op = e->pos();
	painter.moveTo(e->pos());
	if (e->button()==LeftButton)
	    painter.setPen(red);
	else if (e->button()==MidButton)
	    painter.setPen(green);
	else
	    painter.setPen(blue);
    }

    void mouseReleaseEvent(QMouseEvent* e)
    {
	//painter.lineTo(e->pos());
	painter.drawRect(QRect(op,e->pos()));
	repaint(FALSE);
    }

    void paintEvent(QPaintEvent*)
    {
	QPainter p(this);
	//p.fillRect(rect(),blue);
	//p.setClipRegion(clip);
	p.drawImage(0,0,qipd.image());
    }

private:
    QImagePaintDevice32 qipd;
    QPainter painter;
    QRegion clip;
    QPoint op;
    char mode;
    QFont font;
};

main(int argc, char** argv)
{
    QApplication app(argc,argv);
    Main m;
    app.setMainWidget(&m);
    m.show();
    return app.exec();
}
