
#include "qrubberband.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qbitmap.h"
#include "qevent.h"

#include "qrubberband_p.h"
#define d d_func()
#define q q_func()

QRubberBand::QRubberBand(QRubberBand::Type t, QWidget *p) : 
    QWidget(*new QRubberBandPrivate, p, WType_TopLevel | WStyle_Tool | WStyle_Customize | WStyle_NoBorder)
{
    d->type = t;
    if(d->type == Rectangle)
	setAutoMask(true);
}

QRubberBand::~QRubberBand()
{
}

void
QRubberBand::drawRubberBandMask(QPainter *p)
{
    QStyle::SFlags flags = QStyle::Style_Default;
    if(d->type == Rectangle)
	flags |= QStyle::Style_Rectangle;
    style().drawPrimitive(QStyle::PE_RubberBandMask, p, rect(), palette(), flags);
}

void
QRubberBand::drawRubberBand(QPainter *p)
{
    QStyle::SFlags flags = QStyle::Style_Default;
    if(d->type == Rectangle)
	flags |= QStyle::Style_Rectangle;
    style().drawPrimitive(QStyle::PE_RubberBand, p, rect(), palette(), flags);
}

void 
QRubberBand::updateMask()
{
    QBitmap bm(width(), height(), true);
    QPainter p(&bm);
    drawRubberBandMask(&p);
    p.end();
    setMask(QRegion(bm));
}

void
QRubberBand::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawRubberBand(&p);
}

void 
QRubberBand::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange && autoMask()) 
	updateMask();
}
