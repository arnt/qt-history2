/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qbrush.h"
#include "qcleanuphandler.h"
#include "qpixmap.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qdatastream.h"
#include "qdebug.h"

const uchar *qt_patternForBrush(int brushStyle, bool invert)
{
    Q_ASSERT(brushStyle > Qt::SolidPattern && brushStyle < Qt::LinearGradientPattern);
    if(invert) {
        static const uchar dense1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
        static const uchar dense2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
        static const uchar dense3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
        static const uchar dense4_pat[] = { 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 };
        static const uchar dense5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
        static const uchar dense6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
        static const uchar dense7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
        static const uchar hor_pat[]    = { 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00 };
        static const uchar ver_pat[]    = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
        static const uchar cross_pat[]  = { 0x10, 0x10, 0x10, 0xff, 0x10, 0x10, 0x10, 0x10 };
        static const uchar bdiag_pat[]  = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
        static const uchar fdiag_pat[]  = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
        static const uchar dcross_pat[] = { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81 };
        static const uchar *const pat_tbl[] = {
            dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
            dense6_pat, dense7_pat,
            hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };
        return pat_tbl[brushStyle - Qt::Dense1Pattern];
    }
    static const uchar dense1_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static const uchar dense2_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static const uchar dense3_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static const uchar dense4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static const uchar dense5_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static const uchar dense6_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static const uchar dense7_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static const uchar hor_pat[]    = { 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff };
    static const uchar ver_pat[]    = { 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef };
    static const uchar cross_pat[]  = { 0xef, 0xef, 0xef, 0x00, 0xef, 0xef, 0xef, 0xef };
    static const uchar bdiag_pat[]  = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };
    static const uchar fdiag_pat[]  = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
    static const uchar dcross_pat[] = { 0x7e, 0xbd, 0xdb, 0xe7, 0xe7, 0xdb, 0xbd, 0x7e };
    static const uchar *const pat_tbl[] = {
        dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
        dense6_pat, dense7_pat,
        hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };
    return pat_tbl[brushStyle - Qt::Dense1Pattern];
}

QPixmap qt_pixmapForBrush(int brushStyle, bool invert)
{
    QPixmap pm;
    QString key = "$qt-brush$" + QString::number(brushStyle) + QString::number(invert);
    if (!QPixmapCache::find(key, pm)) {
        pm = QBitmap(8, 8, qt_patternForBrush(brushStyle, invert), true);
        QPixmapCache::insert(key, pm);
    }

    return pm;
}


struct QTexturedBrushData : public QBrushData
{
    QPixmap pixmap;
};

struct QLinGradBrushData : public QBrushData
{
    QColor color2;
    QPointF p1;
    QPointF p2;
};


/*!
    \class QBrush qbrush.h

    \brief The QBrush class defines the fill pattern of shapes drawn by a QPainter.

    \ingroup multimedia
    \ingroup shared

    A brush has a style and a color. One of the brush styles is a
    custom pattern, which is defined by a QPixmap.

    The brush style defines the fill pattern. The default brush style
    is \c Qt::NoBrush (depending on how you construct a brush). This style
    tells the painter to not fill shapes. The standard style for
    filling is \c Qt::SolidPattern.

    The brush color defines the color of the fill pattern. The QColor
    documentation lists the predefined colors.

    Use the QPen class for specifying line/outline styles.

    Example:
    \code
        QPainter painter;
        QBrush   brush(yellow);           // yellow solid pattern
        painter.begin(&anyPaintDevice);   // paint something
        painter.setBrush(brush);          // set the yellow brush
        painter.setPen(Qt::NoPen);        // do not draw outline
        painter.drawRect(40,30, 200,100); // draw filled rectangle
        painter.setBrush(Qt::NoBrush);    // do not fill
        painter.setPen(black);            // set black pen, 0 pixel width
        painter.drawRect(10,10, 30,20);   // draw rectangle outline
        painter.end();                    // painting done
    \endcode

    See the setStyle() function for a complete list of brush styles.

    \img brush-styles.png Brush Styles

    \sa QPainter, QPainter::setBrush(), QPainter::setBrushOrigin()
*/



QBrushData *QBrush::shared_default = 0;

/*!
  \internal
  Initializes the brush.
*/

void QBrush::init(const QColor &color, Qt::BrushStyle style)
{
    switch(style) {
    case Qt::TexturePattern:
        d = new QTexturedBrushData;
        static_cast<QTexturedBrushData *>(d)->pixmap = QPixmap();
        break;
    case Qt::LinearGradientPattern:
        d = new QLinGradBrushData;
        break;
    default:
        d = new QBrushData;
        break;
    }
    d->ref = 1;
    d->style = style;
    d->color = color;
}

/*!
    Constructs a default black brush with the style \c Qt::NoBrush (will
    not fill shapes).
*/

QBrush::QBrush()
{
    if (!shared_default) {
        static QCleanupHandler<QBrushData> shared_default_cleanup;
        shared_default = new QBrushData;
        shared_default->ref = 1;
        shared_default->style = (Qt::BrushStyle)0;
        shared_default->color = Qt::black;
        shared_default_cleanup.add(&shared_default);
    }
    d = shared_default;
    ++d->ref;
}

/*!
    Constructs a brush with a black color and a pixmap set to \a pixmap.
*/

QBrush::QBrush(const QPixmap &pixmap)
{
// ## if pixmap was image, we could pick a nice color rather than
// assuming black.
    init(Qt::black, Qt::TexturePattern);
    setTexture(pixmap);
}

/*!
    Constructs a black brush with the style \a style.

    \sa setStyle()
*/

QBrush::QBrush(Qt::BrushStyle style)
{
    init(Qt::black, style);
}

/*!
    Constructs a brush with the color \a color and the style \a style.

    \sa setColor(), setStyle()
*/

QBrush::QBrush(const QColor &color, Qt::BrushStyle style)
{
    init(color, style);
}

/*! \overload
    Constructs a brush with the color \a color and the style \a style.

    \sa setColor(), setStyle()
*/
QBrush::QBrush(Qt::GlobalColor color, Qt::BrushStyle style)
{
    init(color, style);
}

/*!
    Constructs a brush with the color \a color and a custom pattern
    stored in \a pixmap.

    The color will only have an effect for monochrome pixmaps, i.e.
    for QPixmap::depth() == 1.

    Pixmap brushes are currently not supported when printing on X11.

    \sa setColor(), setPixmap()
*/

QBrush::QBrush(const QColor &color, const QPixmap &pixmap)
{
    init(color, Qt::TexturePattern);
    setTexture(pixmap);
}

/*! \overload
    Constructs a brush with the color \a color and a custom pattern
    stored in \a pixmap.

    The color will only have an effect for monochrome pixmaps, i.e.
    for QPixmap::depth() == 1.

    Pixmap brushes are currently not supported when printing on X11.

    \sa setColor(), setPixmap()
*/
QBrush::QBrush(Qt::GlobalColor color, const QPixmap &pixmap)
{
    init(color, Qt::TexturePattern);
    setTexture(pixmap);
}

/*!
    Constructs a brush that is a \link shclass.html shallow
    copy\endlink of \a b.
*/

QBrush::QBrush(const QBrush &b)
{
    d = b.d;
    ++d->ref;
}

/*!
    Creates a linear gradient brush. The brush will interpolate
    between the color \a col1 in the point \a p1 to the color \a col2
    in the point \a p2. The areas outside the interpolation area
    is filled with the color for that corresponding side.
*/

QBrush::QBrush(const QPointF &p1, const QColor &col1, const QPointF &p2, const QColor &col2)
{
    init(col1, Qt::LinearGradientPattern);
    QLinGradBrushData *lgd = static_cast<QLinGradBrushData*>(d);
    lgd->color2 = col2;
    lgd->p1 = p1;
    lgd->p2 = p2;
}


/*!
    Destroys the brush.
*/

QBrush::~QBrush()
{
    if (!--d->ref)
        cleanUp(d);
}

void QBrush::cleanUp(QBrushData *x)
{
    switch (x->style) {
    case Qt::TexturePattern:
        delete static_cast<QTexturedBrushData*>(x);
        break;
    case Qt::LinearGradientPattern:
        delete static_cast<QLinGradBrushData*>(x);
        break;
    default:
        delete x;
    }
}


void QBrush::detach(Qt::BrushStyle newStyle)
{
    if (newStyle == d->style && d->ref == 1)
        return;

    QBrushData *x;
    switch(newStyle) {
    case Qt::TexturePattern:
        x = new QTexturedBrushData;
        static_cast<QTexturedBrushData*>(x)->pixmap =
            d->style == Qt::TexturePattern ? static_cast<QTexturedBrushData *>(d)->pixmap : QPixmap();
        break;
    case Qt::LinearGradientPattern:
        x = new QLinGradBrushData;
        break;
    default:
        x = new QBrushData;
        break;
    }
    x->ref = 1;
    x->style = newStyle;
    x->color = d->color;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        cleanUp(x);
}


/*!
    Assigns \a b to this brush and returns a reference to this brush.
*/

QBrush &QBrush::operator=(const QBrush &b)
{
    QBrushData *x = b.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        cleanUp(x);
    return *this;
}


/*!
    \fn Qt::BrushStyle QBrush::style() const

    Returns the brush style.

    \sa setStyle()
*/

/*!
    Sets the brush style to \a s.

    The brush styles are:
    \table
    \header \i Pattern \i Meaning
    \row \i Qt::NoBrush \i will not fill shapes (default).
    \row \i Qt::SolidPattern  \i solid (100%) fill pattern.
    \row \i Qt::Dense1Pattern \i11 94% fill pattern.
    \row \i Qt::Dense2Pattern \i11 88% fill pattern.
    \row \i Qt::Dense3Pattern \i11 63% fill pattern.
    \row \i Qt::Dense4Pattern \i11 50% fill pattern.
    \row \i Qt::Dense5Pattern \i11 37% fill pattern.
    \row \i Qt::Dense6Pattern \i11 12% fill pattern.
    \row \i Qt::Dense7Pattern \i11 6% fill pattern.
    \row \i Qt::HorPattern \i horizontal lines pattern.
    \row \i Qt::VerPattern \i vertical lines pattern.
    \row \i Qt::CrossPattern \i crossing lines pattern.
    \row \i Qt::BDiagPattern \i diagonal lines (directed /) pattern.
    \row \i Qt::FDiagPattern \i diagonal lines (directed \) pattern.
    \row \i Qt::DiagCrossPattern \i diagonal crossing lines pattern.
    \row \i Qt::TexturePattern \i set when a pixmap pattern is being used.
    \endtable

    On Windows, dense and custom patterns cannot be transparent.

    See the \link #details Detailed Description\endlink for a picture
    of all the styles.

    \sa style()
*/

void QBrush::setStyle(Qt::BrushStyle s)
{
    if (d->style == s)
        return;
    if (s == Qt::TexturePattern)
        qWarning("QBrush::setStyle: TexturePattern is for internal use");
    detach(s);
    d->style = s;
}


/*!
    \fn const QColor &QBrush::color() const

    Returns the brush color.

    \sa setColor()
*/

/*!
    Sets the brush color to \a c.

    \sa color(), setStyle()
*/

void QBrush::setColor(const QColor &c)
{
    detach(d->style);
    d->color = c;
}

/*!
    \fn void QBrush::setColor(Qt::GlobalColor c)

    \overload
*/


#ifdef QT3_SUPPORT

/*!
    \fn void QBrush::setPixmap(const QPixmap &pixmap)

    \compat

    Sets a custom pattern for this brush. Use setTexture() instead.

    \sa setTexture
*/

/*!
    \fn QPixmap *QBrush::pixmap() const

    Returns a pointer to the custom brush pattern, or 0 if no custom
    brush pattern has been set.

    \sa setPixmap()
*/
QPixmap *QBrush::pixmap() const
{
    if (d->style != Qt::TexturePattern)
        return 0;
    QTexturedBrushData *data  = static_cast<QTexturedBrushData*>(d);
    return data->pixmap.isNull() ? 0 : &data->pixmap;
}
#endif

/*!
    \fn QPixmap QBrush::texture() const

    Returns the custom brush pattern, or a null pixmap if no custom brush pattern
    has been set.

    \sa setPixmap()
*/
QPixmap QBrush::texture() const
{
    return d->style == Qt::TexturePattern
                     ? static_cast<const QTexturedBrushData*>(d)->pixmap : QPixmap();
}

/*!
    Sets the brush pixmap to \a pixmap. The style is set to \c
    Qt::TexturePattern.

    The current brush color will only have an effect for monochrome
    pixmaps, i.e. for QPixmap::depth() == 1.

    Pixmap brushes are currently not supported when printing on X11.

    \sa pixmap(), color()
*/

void QBrush::setTexture(const QPixmap &pixmap)
{
    if (!pixmap.isNull()) {
        detach(Qt::TexturePattern);
        QTexturedBrushData *data = static_cast<QTexturedBrushData *>(d);
        data->pixmap = pixmap;
        if (data->pixmap.optimization() == QPixmap::MemoryOptim)
            data->pixmap.setOptimization(QPixmap::NormalOptim);
    } else {
        detach(Qt::NoBrush);
    }
}


/*!
    \fn QColor QBrush::gradientColor() const

    Returns the gradient's secondary color.

    \sa gradientStart() gradientStop()
*/
QColor QBrush::gradientColor() const
{
    return d->style == Qt::LinearGradientPattern
                     ? static_cast<const QLinGradBrushData*>(d)->color2
                     : QColor();
}

/*!
    \fn QPointF QBrush::gradientStart() const

    Returns the gradient's starting color.

    \sa gradientStop() gradientColor()
*/
QPointF QBrush::gradientStart() const
{
    return d->style == Qt::LinearGradientPattern
                     ? static_cast<const QLinGradBrushData*>(d)->p1
                     : QPointF();
}

/*!
    \fn QPointF QBrush::gradientStop() const

    Returns the gradient's ending color.

    \sa gradientStart() gradientColor()
*/
QPointF QBrush::gradientStop() const
{
    return d->style == Qt::LinearGradientPattern
                     ? static_cast<const QLinGradBrushData*>(d)->p2
                     : QPointF();
}



/*!
    \fn bool QBrush::operator!=(const QBrush &b) const

    Returns true if the brush is different from \a b; otherwise
    returns false.

    Two brushes are different if they have different styles, colors or
    pixmaps.

    \sa operator==()
*/

/*!
    Returns true if the brush is equal to \a b; otherwise returns
    false.

    Two brushes are equal if they have equal styles, colors and
    pixmaps.

    \sa operator!=()
*/

bool QBrush::operator==(const QBrush &b) const
{
    if (b.d == d)
        return true;
    if (b.d->style == d->style && b.d->color == d->color) {
        switch (d->style) {
        case Qt::TexturePattern: {
            QPixmap us = static_cast<QTexturedBrushData *>(d)->pixmap;
            QPixmap them = static_cast<QTexturedBrushData *>(b.d)->pixmap;
            return ((us.isNull() && them.isNull()) || us.serialNumber() == them.serialNumber());
        }
        case Qt::LinearGradientPattern: {
            QLinGradBrushData *d1 = static_cast<QLinGradBrushData *>(d);
            QLinGradBrushData *d2 = static_cast<QLinGradBrushData *>(b.d);
            return d1->color2 == d2->color2
                    && d1->p1 == d2->p1
                    && d1->p2 == d2->p2;
        }
        default:
            return true;
        }
    }
    return false;
}

/*!
    \fn QBrush::operator const QColor&() const

    Returns the brush's color.
*/

/*!
    \fn inline double QPainter::translationX() const
    \internal
*/

/*!
    \fn inline double QPainter::translationY() const
    \internal
*/

#ifndef QT_NO_DEBUG_OUTPUT
QDebug operator<<(QDebug dbg, const QBrush &b)
{
#ifndef Q_NO_STREAMING_DEBUG
    dbg.nospace() << "QBrush(" << b.color() << ',' << b.style() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support the streaming of QDebug");
    return dbg;
    Q_UNUSED(b);
#endif
}
#endif

/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QBrush

    Writes the brush \a b to the stream \a s and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QBrush &b)
{
    s << (quint8)b.style() << b.color();
    if (b.style() == Qt::TexturePattern) {
#ifndef QT_NO_IMAGEIO
        s << b.texture();
#else
        qWarning("No Image Brush I/O");
#endif
    } else if (b.style() == Qt::LinearGradientPattern) {
        s << b.gradientColor();
        s << b.gradientStart();
        s << b.gradientStop();
    }
    return s;
}

/*!
    \relates QBrush

    Reads the brush \a b from the stream \a s and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QBrush &b)
{
    quint8 style;
    QColor color;
    s >> style;
    s >> color;
    if (style == Qt::TexturePattern) {
#ifndef QT_NO_IMAGEIO
        QPixmap pm;
        s >> pm;
        b = QBrush(color, pm);
#else
        qWarning("No Image Brush I/O");
#endif
    } else if (style == Qt::LinearGradientPattern) {
        QColor gradientColor;
        QPointF gradientStart, gradientStop;
        s >> gradientColor;
        s >> gradientStart;
        s >> gradientStop;
        b = QBrush(gradientStart, color, gradientStop, gradientColor);
    } else {
        b = QBrush(color, (Qt::BrushStyle)style);
    }
    return s;
}
#endif // QT_NO_DATASTREAM

#if 1
/*******************************************************************************
 * QGradient implementations
 */


/*!
    \class QGradient qbrush.h

    \brief The QGradient class is used in combination with QBrush to
    specify gradient fills.

    Qt currently supports three types of gradient fills: linear,
    radial and conical. Each of these is represented by a subclass of
    QGradient: QLinearGradient, QRadialGradient and QConicalGradient.

    The colors in a gradient is defined using stop points, which is a
    position and a color. The set of stop points describes how the
    gradient area should be filled. A diagonal linear gradient from
    black at (100, 100) to white at (200, 200) could be specified like
    this:

    \code
    QLinearGradient grad(QPointF(100, 100), QPointF(200, 200));
    grad.appendStop(0, Qt::black);
    grad.appendStop(1, Qt::white);
    \endcode

    A gradient can have an arbitrary number of stop points. The
    following gradient would create a radial gradient starting with
    red in the center, blue and then green on the edges:

    \code
    QRadialGradient grad(QPointF(100, 100), 100);
    grad.appendStop(0, Qt::red);
    grad.appendStop(0.5, Qt::blue);
    grad.appendStop(1, Qt::green);
    \endcode

    It is possible to repeat or reflect the gradient outside the area
    by specifiying spread. The default is to pad the outside area with
    the color at the closest stop point.

    \sa QLinearGradient, QRadialGradient, QConicalGradient
*/

/*!
    \internal
*/
QGradient::QGradient()
{
}


/*!
    \enum QGradient::Type

    Specifies the type of gradient.

    \value LinearGradient The gradient is a linear gradient.
    \value RadialGradient The gradient is a radial gradient.
    \value ConicalGradient The gradient is a conical gradient.
*/

/*!
    \enum QGradient::Spread

    Specifies how the areas outside the gradient area should be
    filled.

    \value PadSpread The areas are filled with the closes stop
    color. This is the default.

    \value RepeatSpread The gradient repeats outside the gradient
    area.

    \value ReflectSpread The gradient is reflected outside the
    gradient area.
*/

/*!
    \fn void QGradient::setSpread(QGradient::Spread spread)

    Specifies the spread method that should be used for this
    gradient. This function only has effect for linear and
    radial gradients.
*/

/*!
    \fn QGradient::Spread QGradient::spread() const

    Returns the spread method use by this gradient. The default is
    Pad
*/

/*!
    \fn QGradient::Type QGradient::type() const

    Returns the type of gradient.
*/

/*!
    \overload

    Appends another stop point at the relative position \a pos with
    color \a color. The position \a pos must be in the range 0 to 1
    and must be added in increasing order.
*/

void QGradient::appendStop(qreal pos, const QColor &color)
{
    QGradientStop stop;
    stop.first = pos;
    stop.second = color;
    appendStop(stop);
}


/*!
    Appends the stop point \a stop to the gradient.. The position of \a
    stop must be in the range 0 to 1 and must be added in increasing
    order.
*/

void QGradient::appendStop(const QGradientStop &stop)
{
    if (stop.first > 1 || stop.first < 0) {
        qWarning("QGradient::appendStop(), stop position must be in the range of 0 to 1");
        return;
    } else if (!m_stops.isEmpty() && stop.first <= m_stops.last().first) {
        qWarning("QGradient::appendStop(), stops must be appended in increasing order");
        return;
    }
    m_stops << stop;
}


/*!
    Replaces the current set of stop points with \a stops. The
    positions of the stop points must in the range 0 to 1 and must be
    sorted with the lowest point first.
*/
void QGradient::setStops(const QGradientStops &stops)
{
    m_stops.clear();
    for (int i=0; i<stops.size(); ++i)
        appendStop(stops.at(i));
}


/*!
    Returns the stops for this gradient.

    If no stops have been spesified a gradient of black at 0 to white
    at 1 is used.
*/
QGradientStops QGradient::stops() const
{
    if (m_stops.isEmpty()) {
        QGradientStops tmp;
        tmp << QGradientStop(0, Qt::black) << QGradientStop(1, Qt::white);
        return tmp;
    }
    return m_stops;
}


/*!
    \class QLinearGradient qbrush.h

    \brief The QLinearGradient class is used in combination with QBrush to
    specify a linear gradient brush.

    \sa QBrush
*/


/*!
    Constructs a linear gradient with interpolation area between \a
    start and \a finalStop. The positions \a start and \a finalStop
    are specified using logical coordinates.
*/
QLinearGradient::QLinearGradient(const QPointF &start, const QPointF &finalStop)
{
    m_type = LinearGradient;
    m_spread = PadSpread;
    m_data.linear.x1 = start.x();
    m_data.linear.y1 = start.y();
    m_data.linear.x2 = finalStop.x();
    m_data.linear.y2 = finalStop.y();
}


/*!
    Returns the start point of this linear gradient in logical
    coordinates.
*/

QPointF QLinearGradient::start() const
{
    Q_ASSERT(m_type == LinearGradient);
    return QPointF(m_data.linear.x1, m_data.linear.y1);
}


/*!
    Returns the final stop point of this linear gradient in logical
    coordinates.
*/

QPointF QLinearGradient::finalStop() const
{
    Q_ASSERT(m_type == LinearGradient);
    return QPointF(m_data.linear.x1, m_data.linear.y1);
}


/*!
    \class QRadialGradient qbrush.h

    \breif The QRadialGradient class is used in combination with QBrush to
    specify a radial gradient brush.

    \sa QBrush
*/

/*!
    Constructs a radial gradient centered at \a center with radius \a
    radius.  The \a focalPoint can be used to define the focal point
    of the gradient inside the circle.

    The default focalPoint is the circle center.
*/

QRadialGradient::QRadialGradient(const QPointF &center, qreal radius, const QPointF &focalPoint)
{
    m_type = RadialGradient;
    m_spread = PadSpread;
    m_data.radial.cx = center.x();
    m_data.radial.cy = center.y();
    m_data.radial.fx = focalPoint.x();
    m_data.radial.fy = focalPoint.y();
    m_data.radial.radius = radius;
}


/*!
    Returns the center of this radial gradient in logical coordinates.
*/

QPointF QRadialGradient::center() const
{
    Q_ASSERT(m_type == RadialGradient);
    return QPointF(m_data.radial.cx, m_data.radial.cy);
}


/*!
    Returns the radius of the radial gradient in logical coordinates.
*/

qreal QRadialGradient::radius() const
{
    Q_ASSERT(m_type == RadialGradient);
    return m_data.radial.radius;
}


/*!
    Returns the focal point of this radial gradient in logical
    coordinates.
*/

QPointF QRadialGradient::focalPoint() const
{
    Q_ASSERT(m_type == RadialGradient);
    return QPointF(m_data.radial.fx, m_data.radial.fy);
}


/*!
    \class QConicalGradient qbrush.h

    \brief The QConicalGradient class is used in combination with QBrush to
    specify a conical gradient brush.

    \sa QBrush
*/


/*!
    Constructs a conical centered at \a center and starting at
    \a angle.
*/

QConicalGradient::QConicalGradient(const QPointF &center, qreal angle)
{
    m_type = ConicalGradient;
    m_spread = PadSpread;
    m_data.conical.cx = center.x();
    m_data.conical.cy = center.y();
    m_data.conical.angle = angle;
}


/*!
    Returns the center of the conical gradient in logical coordinates
*/

QPointF QConicalGradient::center() const
{
    Q_ASSERT(m_type == ConicalGradient);
    return QPointF(m_data.conical.cx, m_data.conical.cy);
}


/*!
    Returns the start angle of the conical gradient in logical coordinates
*/

qreal QConicalGradient::angle() const
{
    Q_ASSERT(m_type == ConicalGradient);
    return m_data.conical.angle;
}

#endif
