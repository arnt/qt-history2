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
#include "qdatastream.h"
#include "qdebug.h"

#ifndef Q_WS_WIN
#include "qpixmapcache.h"
#include "qbitmap.h"

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
#endif


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

/*!
    \fn QColor QBrush::gradientColor() const

    Returns the gradient's secondary color.

    \sa gradientStart() gradientStop()
*/


/*!
    \fn QPoint QBrush::gradientStart() const

    Returns the gradient's starting color.

    \sa gradientStop() gradientColor()
*/


/*!
    \fn QPoint QBrush::gradientStop() const

    Returns the gradient's ending color.

    \sa gradientStart() gradientColor()
*/


QBrushData *QBrush::shared_default = 0;

/*!
  \internal
  Initializes the brush.
*/

void QBrush::init(const QColor &color, Qt::BrushStyle style)
{
    switch(style) {
    case Qt::CustomPattern:
        d = new QTexturedBrushData;
        static_cast<QTexturedBrushData *>(d)->pixmap = 0;
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
    init(Qt::black, Qt::CustomPattern);
    setPixmap(pixmap);
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
    init(color, Qt::CustomPattern);
    setPixmap(pixmap);
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
    init(color, Qt::CustomPattern);
    setPixmap(pixmap);
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
    \internal
*/
QBrush::QBrush(const QPoint &p1, const QColor &col1, const QPoint &p2, const QColor &col2)
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
    case Qt::CustomPattern:
        delete static_cast<QTexturedBrushData*>(x)->pixmap;
        delete x;
        break;
    case Qt::LinearGradientPattern:
        delete static_cast<QLinGradBrushData*>(x);
        break;
    default:
        delete x;
    }
}


void QBrush::detach_helper(Qt::BrushStyle newStyle)
{
    QBrushData *x;
    switch(newStyle) {
    case Qt::CustomPattern:
        x = new QTexturedBrushData;
        static_cast<QTexturedBrushData*>(x)->pixmap =
            d->style == Qt::CustomPattern ? static_cast<QTexturedBrushData *>(d)->pixmap : 0;
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
    \row \i Qt::CustomPattern \i set when a pixmap pattern is being used.
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
    if (s == Qt::CustomPattern)
        qWarning("QBrush::setStyle: CustomPattern is for internal use");
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


/*!
    \fn QPixmap *QBrush::pixmap() const

    Returns a pointer to the custom brush pattern, or 0 if no custom
    brush pattern has been set.

    \sa setPixmap()
*/

/*!
    Sets the brush pixmap to \a pixmap. The style is set to \c
    Qt::CustomPattern.

    The current brush color will only have an effect for monochrome
    pixmaps, i.e. for QPixmap::depth() == 1.

    Pixmap brushes are currently not supported when printing on X11.

    \sa pixmap(), color()
*/

void QBrush::setPixmap(const QPixmap &pixmap)
{
    if (!pixmap.isNull()) {
        detach(Qt::CustomPattern);
        QPixmap *pm = new QPixmap(pixmap);
        if (pm->optimization() == QPixmap::MemoryOptim)
            pm->setOptimization(QPixmap::NormalOptim);
        static_cast<QTexturedBrushData *>(d)->pixmap = pm;
    } else {
        detach(Qt::NoBrush);
    }
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
    if (b.d == d || (b.d->style == d->style && b.d->color == d->color)) {
        switch (d->style) {
        case Qt::CustomPattern: {
            QPixmap *us = static_cast<QTexturedBrushData *>(d)->pixmap;
            QPixmap *them = static_cast<QTexturedBrushData *>(b.d)->pixmap;
            return (us == them) || (us && them && us->serialNumber() == them->serialNumber());
        }
        case Qt::LinearGradientPattern:
            return static_cast<QLinGradBrushData*>(d)->color2
                == static_cast<QLinGradBrushData*>(b.d)->color2
                && static_cast<QLinGradBrushData*>(d)->p1
                == static_cast<QLinGradBrushData*>(b.d)->p1
                && static_cast<QLinGradBrushData*>(d)->p2
                == static_cast<QLinGradBrushData*>(b.d)->p2;
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
    \fn QBrush::operator const QPixmap*() const

    Returns the brush's pixmap.
*/

/*!
    \fn inline double QPainter::translationX() const
    \internal
*/

/*!
    \fn inline double QPainter::translationY() const
    \internal
*/

#ifndef QT_NO_DEBUG
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
    s << (Q_UINT8)b.style() << b.color();
    if (b.style() == Qt::CustomPattern)
#ifndef QT_NO_IMAGEIO
        s << *b.pixmap();
#else
        qWarning("No Image Brush I/O");
#endif
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
    Q_UINT8 style;
    QColor color;
    s >> style;
    s >> color;
    if (style == Qt::CustomPattern) {
#ifndef QT_NO_IMAGEIO
        QPixmap pm;
        s >> pm;
        b = QBrush(color, pm);
#else
        qWarning("No Image Brush I/O");
#endif
    } else
        b = QBrush(color, (Qt::BrushStyle)style);
    return s;
}
#endif // QT_NO_DATASTREAM
