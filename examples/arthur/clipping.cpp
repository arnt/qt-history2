#include "clipping.h"

#include <qapplication.h>
#include <qbitmap.h>
#include <qpainter.h>

Clipping::Clipping(QWidget *parent)
    : DemoWidget(parent), textx(0), texty(0)
{
    textdirx = xfunc(1001);
    textdiry = yfunc(1001);
}

void Clipping::paintEvent(QPaintEvent *)
{
    int w = width(), h = height();
    if (bgFill.isNull()) {
        bgFill.resize(w, h);
        QPainter p(&bgFill);

//         if (attributes->antialias)
//             p.setRenderHints(QPainter::LineAntialiasing);

        QColor col1(0, 63, 0);
        QColor col2(255, 255, 100);

        for (int x=0; x<=w; ++x) {
            p.setPen(x % 2 ? col1 : col2);
            p.drawLine(x, 0, w-x, h);
        }
        for (int y=0; y<=h; ++y) {
            p.setPen(y % 2 ? col1 : col2);
            p.drawLine(0, y, w, h-y);
        }
        p.end();
    }

    QPainter pt(this);
    pt.drawPixmap(0, 0, w, h, bgFill);

    double x = xfunc(animationStep);
    double y = yfunc(animationStep);

    QBitmap bm(w, h);
    bm.fill(Qt::color1);
    QPainter bmPainter(&bm);

    bmPainter.setBackgroundMode(Qt::TransparentMode);

    bmPainter.setBrush(Qt::color0);
    bmPainter.setPen(Qt::color0);

    bmPainter.save();
    bmPainter.translate(w/2, h/2);
    bmPainter.rotate(animationStep);
    bmPainter.drawRect(-100 -100*x, -100 -100*y, 200 + 200*x, 200 + 200*y);
    bmPainter.restore();

    QRect br;
    QString text = "Arthur\nThe Paint Engine";
    QFont font = qApp->font();
    font.setPointSize(36);
    font.setBold(true);
    bmPainter.setFont(font);
    QFontMetrics fm = bmPainter.fontMetrics();
    bmPainter.drawText(0, 0, w, h, Qt::AlignCenter | Qt::TextDontPrint, text, -1, &br);

    const int speed = 5;

    textx += int(textdirx*speed);
    if (textx < 0)
        textdirx = -textdirx;
    else if (textx > br.x() + br.width())
        textdirx = -textdirx;

    texty += int(textdiry*speed);
    if (texty < 0)
        textdiry = -textdiry;
    else if (texty > br.y() + br.height())
        textdiry = -textdiry;

    bmPainter.drawText(textx, texty, br.width(), br.height(), Qt::AlignCenter, text);

    bmPainter.end();

    pt.setClipRegion(bm);
    fillBackground(&pt);

}

void Clipping::resizeEvent(QResizeEvent *)
{
    bgFill = QPixmap();
}
