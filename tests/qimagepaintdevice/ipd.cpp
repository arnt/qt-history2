#include "qimagepaintdevice.h"
#include <qapplication.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


class Image {
public:

    void help() 
    {
    	QString help =
	    "h = Help\n"
	    "q = Quit\n\n"
	    "l = Lines\n"
	    "l = Rectangles\n"
	    "p = Polygons\n"
	    "b = Beziers\n"
	    "i = Images (1,2,3 changes image)\n"
	    "t = Text (1,2,3 changes font)";
	QRect r = painter.boundingRect(0,0,0,0,0,help);
	r.moveBy(50,50);
	painter.fillRect(r,Qt::white);
	painter.setPen(Qt::black);
	painter.drawText(r,0,help);
    }
    Image(bool fullscreen)
    {
	if (fullscreen) {
	    qipd = new QImagePaintDevice32(QImagePaintDevice32::XFreeDGA);
	    widget = 0;
	} else {
	    qipd = new QImagePaintDevice32(640,480);
	    widget = new QWidget;
	    widget->resize(qipd->image().width(),qipd->image().height());
	    qApp->setMainWidget(widget);
	    widget->show();
	}

	painter.begin(qipd);
	clip = QRect(0,0,qipd->image().width(),qipd->image().height());
	mode = 'I';
	painter.setClipRegion(clip);
	QImage bg("bg.png");
	painter.drawImage(0,0,bg);
	help();
	drawme = QImage("pngtest.png").convertDepth(32);
	showImage();
    }

    ~Image()
    {
	painter.end();
	delete qipd;
    }

    void doAscii(char ascii)
    {
	char shape=0;


	switch ( ascii ) {
	  case 'q':
	    qApp->exit();
	    break;
	  case 'h':
	      help();
	      showImage();
	      break;
	  case 'c':
	    for ( int i=0; i<100; i++ ) {
		QRect rnd(rand()%qipd->image().width(),rand()%qipd->image().height(),rand()%50,rand()%50);
		clip -= rnd;
	    }
	    painter.setClipRegion(clip);
	    break;
	  case 'l':
	  case 'r':
	  case 'p':
	  case 't':
	  case 'i':
	  case 'b':
	    shape = ascii;
	    break;
	  case 'S':
	  case 'I':
	  case 'P':
	    mode = ascii;
	    break;
	  case '1':
	    font = QFont("Helvetica",12);
	    drawme = QImage("pngtest.png").convertDepth(32);
	    break;
	  case '2':
	    font = QFont("Verdana",12);
	    drawme = QImage("trans1.png").convertDepth(32);
	    break;
	  case '3':
	    font = QFont("Verdana",72);
	    drawme = QImage("bg.png").convertDepth(32);
	    break;
	}

	if ( mode && shape ) {
	    QPainter *ptr=0;

	    QPixmap pm(qipd->image().size());
	    QPainter ppm(&pm);
	    if ( mode == 'P' ) {
		ppm.setBrush(Qt::cyan);
		ppm.setClipRegion(clip);
		ptr = &ppm;
	    }

	    QPainter px11(widget);
	    if ( mode == 'S' ) {
		px11.setBrush(Qt::yellow);
		px11.setClipRegion(clip);
		ptr = &px11;
	    }

	    painter.setPen(Qt::magenta);
	    if ( mode == 'I' ) {
		ptr = &painter;
	    }

	    if ( shape=='l' || shape=='t' ) {
		painter.setPen(Qt::magenta);
		px11.setPen(Qt::yellow);
		ppm.setPen(Qt::cyan);
	    } else {
		painter.setPen(Qt::white);
		px11.setPen(Qt::NoPen);
		ppm.setPen(Qt::NoPen);
		px11.setBrush(Qt::blue);
	    }

	    QPointArray poly(50);
	    srand(1234);
	    if ( shape == 'p' ) {
		for (int p=0; p<50; p++) {
		    poly[p] = QPoint(rand()%qipd->image().width()/2,rand()%qipd->image().height()/2);
		}
	    } else if ( shape == 't' ) {
		ptr->setFont(font);
	    } else if ( shape == 'b' ) {
		poly[0] = QPoint(0,0);
		poly[1] = QPoint(10,50);
		poly[2] = QPoint(-50,50);
		poly[3] = QPoint(100,50);
	    }
	    QTime t;
	    t.start();
	    int x=0,y=0;
	    int w=qipd->image().width(),h=qipd->image().height();
	    int n=shape == 'l'||shape=='b' ? 5000 : 500;
	    long pix=0;
	    for (int i=0; i<n; i++) {
		if ( n < 2000 || i%16==0 ) {
		    QColor c(rand()%256,rand()%256,rand()%256);
		    ptr->setBrush(c);
		    ptr->setPen(c);
		}
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
		} else if ( shape=='b' ) {
		    poly.translate(x,y);
		    ptr->drawQuadBezier(poly);
		    poly.translate(-x,-y);
		    x += 12345*i;
		    y += 2345+i;
		    x %= w;
		    y %= h;
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
		    ptr->drawText(0,0,"Top");
		    ptr->drawText(0,479,"Bottomy");
		    x += 12345*i;
		    y += 2345+i;
		    x %= w;
		    y %= h;
		} else if ( shape=='i' ) {
		    ptr->drawImage(x,y,drawme);
		    x += 12345*i;
		    y += 2345+i;
		    x %= w;
		    y %= h;
		    pix+=drawme.width()*drawme.height();
		}
	    }
	    qApp->syncX();
	    int el = t.elapsed();
	    qDebug("%s%c: %dms for %d %c's (%ld pixels/second)",
#ifdef QT_DEBUG
		"Unoptimized: ",
#else
		"",
#endif
		mode, el,n,shape, long(el ? pix/el*1000 : 9999999));
	    showImage();
	}
    }
    void showImage()
    {
	if ( widget ) {
	    QPixmap b;
	    b.convertFromImage(qipd->image());
	    widget->setBackgroundPixmap(b);
	}
    }

private:
    QImagePaintDevice32* qipd;
    QImage drawme;
    QWidget* widget;
    QPainter painter;
    QRegion clip;
    QPoint op;
    char mode;
    QFont font;
};

Image* gi;
class MyQApplication : public QApplication {
public:
    MyQApplication(int& argc, char** argv) :
	QApplication(argc,argv)
    {
    }

    bool x11EventFilter( XEvent* e )
    {
	if ( e->type == 2 ) {
	    char buf[3];
	    KeySym ks = 0;
	    XLookupString(&e->xkey, buf, 2, &ks, NULL);
	    gi->doAscii(buf[0]);
	    return TRUE;
	} else {
	    return QApplication::x11EventFilter(e);
	}
    }
};

main(int argc, char** argv)
{
    MyQApplication app(argc,argv);
    Image i(argc > 1);
    gi = &i;
    return app.exec();
}

