
#include "qrubberband.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qbitmap.h"
#include "qevent.h"

#include <private/qwidget_p.h>
class QRubberBandPrivate : public QWidgetPrivate
{
public:
    QRubberBand::Shape shape;
};
#define d d_func()
#define q q_func()

/*!
   \class QRubberBand qrubberband.h
   \brief The QRubberBand class provides a rectangle or line that can
   indicate a selection or a boundary.

    \ingroup misc
    \mainclass

    A rubber band is often used to show a new bounding area (as in a
    QSplitter or a QDockWindow that is undocking). Commonly this has
    been implemented using a QPainter and XOR, but this approach
    doesn't always work properly since rendering can happen in the
    window below the rubber band, but before the rubber band has been
    "erased".

    You can create a QRubberBand whenever you need to render a rubber
    band around a given area (or to represent a single line), then
    call setGeometry() to move and size it; hiding the widget will
    make the rubber band disappear.
### Why hide it rather than delete it?
*/

//### How about some nice convenience constructors?
//QRubberBand::QRubberBand(QRubberBand::Type t, const QRect &rect, QWidget *p)
//QRubberBand::QRubberBand(QRubberBand::Type t, int x, int y, int w, int h, QWidget *p)

/*!
    Constructs a rubber band of shape \a s, with parent \a p.

    By default a rectangular QRubberBand (\a t is \c Rectangle) will
    be set to auto mask, so that the boundry of the rectangle is all
    that is visible. Some styles (for example native Mac OS X) will
    change this and call QWidget::setWindowOpacity() to make the
    window only partially opaque.
*/
QRubberBand::QRubberBand(QRubberBand::Shape s, QWidget *p) : 
    QWidget(*new QRubberBandPrivate, p, WType_TopLevel | WStyle_Tool | WStyle_Customize | WStyle_NoBorder)
{
    d->shape = s;
    if(d->shape == Rectangle)
	setAutoMask(true);
}

/*!
  Destructor.
*/
QRubberBand::~QRubberBand()
{
}

/*!
    \enum QRubberBand::Shape

    \value Line
    \value Rectangle
*/

/*!
    Virtual function that draws the mask of a QRubberBand using
    painter \a p.

    The drawing is themed (using QStyle), but you can reimplement it
    to achieve custom effects.
*/
void
QRubberBand::drawRubberBandMask(QPainter *p)
{
    QStyle::SFlags flags = QStyle::Style_Default;
    if(d->shape == Rectangle)
	flags |= QStyle::Style_Rectangle;
    style().drawPrimitive(QStyle::PE_RubberBandMask, p, rect(), palette(), flags);
}

/*!
    Virtual function that draws the contents of a QRubberBand using
    painter \a p.

    The drawing is themed (using QStyle), but you can reimplement it
    to achieve custom effects.
*/
void
QRubberBand::drawRubberBand(QPainter *p)
{
    QStyle::SFlags flags = QStyle::Style_Default;
    if(d->shape == Rectangle)
	flags |= QStyle::Style_Rectangle;
    style().drawPrimitive(QStyle::PE_RubberBand, p, rect(), palette(), flags);
}

/*!
  Returns the shape of this rubber band. The shape can only be set
  upon construction.
*/
QRubberBand::Shape 
QRubberBand::shape() const 
{ 
    return d->shape; 
}

/*!
    \reimp
*/
void 
QRubberBand::updateMask()
{
    QBitmap bm(width(), height(), true);
    QPainter p(&bm);
    drawRubberBandMask(&p);
    p.end();
    setMask(QRegion(bm));
}

/*!
    \reimp
*/
void
QRubberBand::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawRubberBand(&p);
}

/*!
    \reimp
*/
void 
QRubberBand::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange && autoMask()) 
	updateMask();
}
