/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qdirectpainter_qws.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qevent.h>

class DirectPainterExample : public QWidget {
public:
    DirectPainterExample()
    {
	startTimer(20);
    }

    void timerEvent(QTimerEvent*)
    {
	b = (b+1)&0xff;
	draw(rect());
    }

    void paintEvent(QPaintEvent* e)
    {
	draw(e->rect());
    }

private:
    void draw(const QRect& cr)
    {
	QDirectPainter p(this);
	if ( p.depth() == 32 && p.transformOrientation() == 0 ) {
	    QRect dcr = cr;
	    dcr.moveBy(p.xOffset(),p.yOffset());
	    for (int i=p.numRects(); i--; ) {
		QRect dr = p.rect(i) & dcr;
		QRect cdr = dr & dcr;
		if ( !cdr.isEmpty() ) {
		    int r = cdr.y()-p.yOffset();
		    int ig = cdr.x()-p.xOffset();
		    uchar* fb = p.frameBuffer()
			+ (cdr.y())*p.lineStep()
			+ (cdr.x())*4;
		    for (int y=0; y<cdr.height(); y++) {
			QRgb* l = (QRgb*)fb;
			int g = ig;
			for (int x=cdr.width(); x--; ) {
			    *l++ = qRgb(r,g,b);
			    g++;
			}
			fb += p.lineStep();
			r++;
		    }
		}
	    }
	} else {
	    // a fallback implementation
	    p.fillRect(cr, QColor(0,0,b));
	}
    }

    int b;
};

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    DirectPainterExample m;
    a.setMainWidget( &m );
    m.show();
    return a.exec();
}
