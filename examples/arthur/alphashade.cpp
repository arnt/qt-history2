#include "alphashade.h"

#include <qpainter.h>

AlphaShade::AlphaShade(QWidget *parent)
    : DemoWidget(parent),
      iterations(2),
      spread(5)
{
}

void AlphaShade::paintEvent(QPaintEvent *)
{
    if (!attributes)
        return;

    QPainter p(this);
    fillBackground(&p);

    int w = width(), h = height();

    // Get painter into position...
    p.translate(w/2-10, h/2-20);
    p.rotate(animationStep);
    p.translate(-w/4, -h/4);
    p.setPen(Qt::NoPen);

    if (attributes->antialias)
        p.setRenderHints(QPainter::LineAntialiasing);

    int offset = h / 10;

    // Transparent shadow...
    p.setBrush(QColor(0, 0, 0, attributes->alpha ? 127/iterations/iterations : 255));
    for (int x=0; x<iterations*spread; x+=spread) {
        for (int y=0; y<iterations*spread; y+=spread) {
            p.save();
            p.rotate(-animationStep);
            p.translate(offset + x, offset * 1.5 + y);
            p.rotate(animationStep);
            p.drawRect(0, 0, w/2, h/2);
            p.restore();
        }
    }

    // The solid fill on top...
    p.setPen(Qt::black);
    p.setBrush(QColor(255, 255, 255, attributes->alpha ? 127 : 255 ));
    p.drawRect(0, 0, w/2, h/2);
}

QString AlphaShade::description() const
{
    return "AlphaShade displays semitransparent shadow of a rotation rectangle."
        " Alpha blended primitives are now availalbe in Qt.";
}
