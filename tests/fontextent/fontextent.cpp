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
    flags=AlignVCenter|AlignLeft;
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
	flags = AlignVCenter|AlignLeft;
    } else if ( ke->key() == Key_Right ) {
	flags = AlignVCenter|AlignRight;
    } else if ( ke->key() == Key_Up ||  ke->key() == Key_Down ) {
	flags = AlignCenter;
    } else if ( ke->key() == Key_Escape ) {
	QLabel *l = new QLabel(line);
	l->resize(l->sizeHint());
	l->setAlignment(flags);
	l->show();
    } else if ( ke->key() == Key_G && ke->state()&AltButton ) {
	flags ^= GrayText;
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
    QRect pbr = p.boundingRect(r,flags,line);
    int right = fm.width(line)+hmarg-1;

    p.setPen(yellow);
    p.drawLine(hmarg,0,hmarg,height());
    p.drawLine(0,vmarg,width(),vmarg);
    p.drawLine(0,vmarg+h,width(),vmarg+h);

    p.setPen(black);
    p.drawRect(br);
    p.setPen(green);
    p.drawLine(hmarg, br.bottom(), hmarg, br.bottom()+10);
    p.drawLine(right, br.bottom(), right, br.bottom()+10);
    if (line.length()==1) {
	QString str;
	str.sprintf("%c: lb = %d, rb = %d, width=%d\n"
		    "    ml = %d, mr = %d",
	    line[0],
	    fm.leftBearing(line[0]),
	    fm.rightBearing(line[0]),
	    fm.width(line[0]),
	    fm.minLeftBearing(),
	    fm.minRightBearing());
	QFont f = p.font();
	QFontMetrics fm = p.fontMetrics();
	p.setFont(QFont("Helvetica", 10));
	p.drawText(hmarg, br.bottom()+fm.lineSpacing()+10, str);
	p.setFont(f);
    }
    p.setPen(blue);
    p.drawText(hmarg,vmarg,line);

    p.setPen(yellow);
    p.drawRect(r);
    p.setPen(magenta);
    p.drawRect(pbr);
    p.setPen(blue);
    p.drawText(pbr,flags,line);
}

main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setFont(QFont("Charter", 100, QFont::Normal, TRUE));

    Main m;
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
