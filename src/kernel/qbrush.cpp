/****************************************************************************
**
** Implementation of QPainter, QPen and QBrush classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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

/*!
    \class QBrush qbrush.h

    \brief The QBrush class defines the fill pattern of shapes drawn by a QPainter.

    \ingroup graphics
    \ingroup images
    \ingroup shared

    A brush has a style and a color. One of the brush styles is a
    custom pattern, which is defined by a QPixmap.

    The brush style defines the fill pattern. The default brush style
    is \c NoBrush (depending on how you construct a brush). This style
    tells the painter to not fill shapes. The standard style for
    filling is \c SolidPattern.

    The brush color defines the color of the fill pattern. The QColor
    documentation lists the predefined colors.

    Use the QPen class for specifying line/outline styles.

    Example:
    \code
        QPainter painter;
        QBrush   brush( yellow );           // yellow solid pattern
        painter.begin( &anyPaintDevice );   // paint something
        painter.setBrush( brush );          // set the yellow brush
        painter.setPen( NoPen );            // do not draw outline
        painter.drawRect( 40,30, 200,100 ); // draw filled rectangle
        painter.setBrush( NoBrush );        // do not fill
        painter.setPen( black );            // set black pen, 0 pixel width
        painter.drawRect( 10,10, 30,20 );   // draw rectangle outline
        painter.end();                      // painting done
    \endcode

    See the setStyle() function for a complete list of brush styles.

    \img brush-styles.png Brush Styles

    \sa QPainter, QPainter::setBrush(), QPainter::setBrushOrigin()
*/


QBrush::QBrushData *QBrush::shared_default = 0;

/*!
  \internal
  Initializes the brush.
*/

void QBrush::init(const QColor &color, BrushStyle style)
{
    d = new QBrushData;
    d->ref = 1;
    d->style = style;
    d->color = color;
    d->pixmap = 0;
}

/*!
    Constructs a default black brush with the style \c NoBrush (will
    not fill shapes).
*/

QBrush::QBrush()
{
    if ( !shared_default ) {
	static QCleanupHandler<QBrush::QBrushData> shared_default_cleanup;
	shared_default = new QBrushData;
	shared_default->ref = 1;
	shared_default->style = (BrushStyle)0;
	shared_default->color = black;
	shared_default->pixmap = 0;
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
    init(Qt::black, CustomPattern);
    setPixmap(pixmap);
}

/*!
    Constructs a black brush with the style \a style.

    \sa setStyle()
*/

QBrush::QBrush(BrushStyle style)
{
    init(black, style);
}

/*!
    Constructs a brush with the color \a color and the style \a style.

    \sa setColor(), setStyle()
*/

QBrush::QBrush(const QColor &color, BrushStyle style)
{
    init(color, style);
}

/*! \overload
    Constructs a brush with the color \a color and the style \a style.

    \sa setColor(), setStyle()
*/
QBrush::QBrush(Qt::GlobalColor color, BrushStyle style)
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
    init(color, CustomPattern);
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
    init(color, CustomPattern);
    setPixmap(pixmap);
}

/*!
    Constructs a brush that is a \link shclass.html shallow
    copy\endlink of \a b.
*/

QBrush::QBrush( const QBrush &b )
{
    d = b.d;
    ++d->ref;
}

/*!
    Destroys the brush.
*/

QBrush::~QBrush()
{
    if (!--d->ref)
	cleanUp(d);
}

void QBrush::cleanUp(QBrush::QBrushData *x)
{
    delete x->pixmap;
    delete x;
}


void QBrush::detach_helper()
{
    QBrushData *x = new QBrushData;
    x->ref = 1;
    x->style = d->style;
    x->color = d->color;
    if (d->pixmap)
	x->pixmap = new QPixmap(*d->pixmap);
    else
	x->pixmap = 0;
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
    \fn BrushStyle QBrush::style() const

    Returns the brush style.

    \sa setStyle()
*/

/*!
    Sets the brush style to \a s.

    The brush styles are:
    \table
    \header \i Pattern \i Meaning
    \row \i NoBrush \i will not fill shapes (default).
    \row \i SolidPattern  \i solid (100%) fill pattern.
    \row \i Dense1Pattern \i11 94% fill pattern.
    \row \i Dense2Pattern \i11 88% fill pattern.
    \row \i Dense3Pattern \i11 63% fill pattern.
    \row \i Dense4Pattern \i11 50% fill pattern.
    \row \i Dense5Pattern \i11 37% fill pattern.
    \row \i Dense6Pattern \i11 12% fill pattern.
    \row \i Dense7Pattern \i11 6% fill pattern.
    \row \i HorPattern \i horizontal lines pattern.
    \row \i VerPattern \i vertical lines pattern.
    \row \i CrossPattern \i crossing lines pattern.
    \row \i BDiagPattern \i diagonal lines (directed /) pattern.
    \row \i FDiagPattern \i diagonal lines (directed \) pattern.
    \row \i DiagCrossPattern \i diagonal crossing lines pattern.
    \row \i CustomPattern \i set when a pixmap pattern is being used.
    \endtable

    On Windows, dense and custom patterns cannot be transparent.

    See the \link #details Detailed Description\endlink for a picture
    of all the styles.

    \sa style()
*/

void QBrush::setStyle(BrushStyle s)
{
    if (d->style == s)
	return;
    if (s == CustomPattern)
	qWarning( "QBrush::setStyle: CustomPattern is for internal use" );
    detach();
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
    detach();
    d->color = c;
}


/*!
    \fn QPixmap *QBrush::pixmap() const

    Returns a pointer to the custom brush pattern, or 0 if no custom
    brush pattern has been set.

    \sa setPixmap()
*/

/*!
    Sets the brush pixmap to \a pixmap. The style is set to \c
    CustomPattern.

    The current brush color will only have an effect for monochrome
    pixmaps, i.e. for QPixmap::depth() == 1.

    Pixmap brushes are currently not supported when printing on X11.

    \sa pixmap(), color()
*/

void QBrush::setPixmap(const QPixmap &pixmap)
{
    detach();
    if (d->pixmap)
	delete d->pixmap;
    if (pixmap.isNull()) {
	d->style  = NoBrush;
	d->pixmap = 0;
    } else {
	d->style = CustomPattern;
	d->pixmap = new QPixmap(pixmap);
	if (d->pixmap->optimization() == QPixmap::MemoryOptim)
	    d->pixmap->setOptimization(QPixmap::NormalOptim);
    }
}


/*!
    \fn bool QBrush::operator!=( const QBrush &b ) const

    Returns TRUE if the brush is different from \a b; otherwise
    returns FALSE.

    Two brushes are different if they have different styles, colors or
    pixmaps.

    \sa operator==()
*/

/*!
    Returns TRUE if the brush is equal to \a b; otherwise returns
    FALSE.

    Two brushes are equal if they have equal styles, colors and
    pixmaps.

    \sa operator!=()
*/

bool QBrush::operator==(const QBrush &b) const
{
    return (b.d == d)
	   || (b.d->style == d->style && b.d->color == d->color && b.d->pixmap == d->pixmap);
}


/*!
    \fn inline double QPainter::translationX() const
    \internal
*/

/*!
    \fn inline double QPainter::translationY() const
    \internal
*/

#if !defined(Q_OS_MAC) || QT_MACOSX_VERSION >= 0x1030
#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QBrush &b)
{
    dbg.nospace() << "QBrush(" << b.color() << ',' << b.style() << ')';
    return dbg.space();
}
#endif
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
