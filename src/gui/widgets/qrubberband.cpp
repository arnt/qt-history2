
#include "qrubberband.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qbitmap.h"
#include "qevent.h"

#include "qrubberband_p.h"
#define d d_func()
#define q q_func()

/*!
   \class QRubberBand qrubberband.h
   \brief The QRubberBand widget provides a widget that can shown to represent a rubber band
    around selection (or temporary boundries).

    \ingroup misc
    \mainclass

    A rubber band is often displayed to show a new bounding area (as
    in a QSplitter or a QDockWindow that is undocking). Traditionally
    this has been implemented using a QPainter and XOr, this can be
    dangerous as rendering can happen in the window below the rubber
    band, but before the rubber band has been "erased". 

    You can create a QRubberBand class whenever you need to render a
    rubber band around a given area (or to represent a single line),
    then setGeometry() on it to move to a new rectangle, finally
    hiding the widget will make the rubber band disappear. 

*/


/*!
    Construct a rubber band of type \a t.
  
    By default a \e Rectangle QRubberBand will be set to auto mask, so
    that the boundry of the rectangle is all that is visible, some
    styles (for example native Mac OS X) will change this and call
    QWidget::setWindowOpacity() to make the window only partially
    opaque.
*/
QRubberBand::QRubberBand(QRubberBand::Type t, QWidget *p) : 
    QWidget(*new QRubberBandPrivate, p, WType_TopLevel | WStyle_Tool | WStyle_Customize | WStyle_NoBorder)
{
    d->type = t;
    if(d->type == Rectangle)
	setAutoMask(true);
}

/*!
  Destructor.
*/
QRubberBand::~QRubberBand()
{
}

/*!
    Virtual function that draws the mask of a QRubberBand.

   This is normally themed (via QStyle) however you can overload this
   for special renderings in a given widget.
*/
void
QRubberBand::drawRubberBandMask(QPainter *p)
{
    QStyle::SFlags flags = QStyle::Style_Default;
    if(d->type == Rectangle)
	flags |= QStyle::Style_Rectangle;
    style().drawPrimitive(QStyle::PE_RubberBandMask, p, rect(), palette(), flags);
}

/*!
    Virtual function that draws the contents of a QRubberBand.

   This is normally themed (via QStyle) however you can overload this
   for special renderings in a given widget.
*/
void
QRubberBand::drawRubberBand(QPainter *p)
{
    QStyle::SFlags flags = QStyle::Style_Default;
    if(d->type == Rectangle)
	flags |= QStyle::Style_Rectangle;
    style().drawPrimitive(QStyle::PE_RubberBand, p, rect(), palette(), flags);
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
