#include "fontextent.h"
#include <qpainter.h>
#include <qapp.h>
#include <qkeycode.h>
#include <qlabel.h>
#include <qprinter.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    line="type";
    align=AlignVCenter|AlignLeft;
    setBackgroundColor(white);
}

void Main::bang()
{
}

void Main::resizeEvent(QResizeEvent*)
{
}

void Main::keyPressEvent(QKeyEvent* ke)
{
    if ( ke->key() == Key_Backspace ) {
	if ( line.length() )
	    line = line.left(line.length()-1);
    } else if ( ke->key() == Key_Return ) {
	line += '\n';
    } else if ( ke->key() == Key_Left ) {
	align = AlignVCenter|AlignLeft;
    } else if ( ke->key() == Key_Right ) {
	align = AlignVCenter|AlignRight;
    } else if ( ke->key() == Key_Up ||  ke->key() == Key_Down ) {
	align = AlignCenter;
    } else if ( ke->key() == Key_Escape ) {
	QLabel *l = new QLabel(line);
	l->resize(l->sizeHint());
	l->setAlignment(align);
	l->show();
    } else if ( ke->key() == Key_P && ke->state()&AltButton ) {
	QPrinter prn;
	if (prn.setup(this)) {
	    QPainter p(&prn);
	    draw(p);
	}
    } else {
	line += ke->ascii();
    }
    update();
}

void Main::keyReleaseEvent(QKeyEvent*)
{
}

void Main::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    draw(p);
}

void Main::draw(QPainter& p)
{
    QFontMetrics fm = p.fontMetrics();
    QRect br = fm.boundingRect(line);
    int w = width()/2;
    int h = height()/2;
    int vmarg = h*3/4;
    int hmarg = w/2;
    br.moveBy(hmarg,vmarg);
    QRect r(hmarg,vmarg+h-80,w/2,80);
    QRect pbr = p.boundingRect(r,align,line);

    p.setPen(yellow);
    p.drawLine(hmarg,0,hmarg,height());
    p.drawLine(0,vmarg,width(),vmarg);
    p.drawLine(0,vmarg+h,width(),vmarg+h);

    p.setPen(black);
    p.drawRect(br);
    p.setPen(blue);
    p.drawText(hmarg,vmarg,line);

    p.setPen(yellow);
    p.drawRect(r);
    p.setPen(magenta);
    p.drawRect(pbr);
    p.setPen(blue);
    p.drawText(pbr,align,line);
}

main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setFont(QFont("Times", 100, QFont::Normal, TRUE));

    Main m;
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
