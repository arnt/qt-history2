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

#include "qpen.h"
#include "qdatastream.h"

/*!
    \class QPen qpen.h
    \brief The QPen class defines how a QPainter should draw lines and outlines
    of shapes.

    \ingroup multimedia
    \ingroup shared
    \mainclass

    A pen has a style, width, color, cap style and join style.

    The pen style defines the line type. The default pen style is \c
    Qt::SolidLine. Setting the style to \c Qt::NoPen tells the painter to
    not draw lines or outlines.

    When drawing 1 pixel wide diagonal lines you can either use a very
    fast algorithm (specified by a line width of 0, which is the
    default), or a slower but more accurate algorithm (specified by a
    line width of 1). For horizontal and vertical lines a line width
    of 0 is the same as a line width of 1. The cap and join style have
    no effect on 0-width lines.

    The pen color defines the color of lines and text. The default
    line color is black. The QColor documentation lists predefined
    colors.

    The cap style defines how the end points of lines are drawn. The
    join style defines how the joins between two lines are drawn when
    multiple connected lines are drawn (QPainter::drawPolyline()
    etc.). The cap and join styles only apply to wide lines, i.e. when
    the width is 1 or greater.

    Use the QBrush class to specify fill styles.

    Example:
    \code
    QPainter painter;
    QPen pen(red, 2);                 // red solid line, 2 pixels wide
    painter.begin(&anyPaintDevice);   // paint something
    painter.setPen(pen);              // set the red, wide pen
    painter.drawRect(40,30, 200,100); // draw a rectangle
    painter.setPen(blue);             // set blue pen, 0 pixel width
    painter.drawLine(40,30, 240,130); // draw a diagonal in rectangle
    painter.end();                    // painting done
    \endcode

    See the \l Qt::PenStyle enum type for a complete list of pen
    styles.

    With reference to the end points of lines, for wide (non-0-width)
    pens it depends on the cap style whether the end point is drawn or
    not. QPainter will try to make sure that the end point is drawn
    for 0-width pens, but this cannot be absolutely guaranteed because
    the underlying drawing engine is free to use any (typically
    accelerated) algorithm for drawing 0-width lines. On all tested
    systems, however, the end point of at least all non-diagonal lines
    are drawn.

    A pen's color(), width(), style(), capStyle() and joinStyle() can
    be set in the constructor or later with setColor(), setWidth(),
    setStyle(), setCapStyle() and setJoinStyle(). Pens may also be
    compared and streamed.

    \img pen-styles.png Pen styles

    \sa QPainter, QPainter::setPen()
*/

/*!
  \internal
  Initializes the pen.
*/


void QPen::init(const QColor &color, float width, uint linestyle)
{
    d = new QPenData;
    d->ref = 1;
    d->style = (Qt::PenStyle)(linestyle & Qt::MPenStyle);
    d->width = width;
    d->color = color;
    d->linest = linestyle;
}

/*!
    Constructs a default black solid line pen with 0 width, which
    renders lines 1 pixel wide (fast diagonals).
*/

QPen::QPen()
{
    init(Qt::black, 0, Qt::SolidLine | Qt::SquareCap);
}

/*!
    Constructs a black pen with 0 width (fast diagonals) and style \a
    style.

    \sa setStyle()
*/

QPen::QPen(Qt::PenStyle style)
{
    init(Qt::black, 0, style | Qt::SquareCap);
}

/*!
    Constructs a pen with the specified \a color, \a width and \a
    style.

    \sa setWidth(), setStyle(), setColor()
*/

QPen::QPen(const QColor &color, float width, Qt::PenStyle style)
{
    init(color, width, style | Qt::SquareCap);
}

/*!
    Constructs a pen with the specified color \a cl and width \a width.
    The pen style is set to \a s, the pen cap style to \a c and the
    pen join style to \a j.

    A line width of 0 will produce a 1 pixel wide line using a fast
    algorithm for diagonals. A line width of 1 will also produce a 1
    pixel wide line, but uses a slower more accurate algorithm for
    diagonals. For horizontal and vertical lines a line width of 0 is
    the same as a line width of 1. The cap and join style have no
    effect on 0-width lines.

    \sa setWidth(), setStyle(), setColor()
*/

QPen::QPen(const QColor &cl, float width, Qt::PenStyle s, Qt::PenCapStyle c, Qt::PenJoinStyle j)
{
    init(cl, width, s | c | j);
}

/*!
    Constructs a pen that is a copy of \a p.
*/

QPen::QPen(const QPen &p)
{
    d = p.d;
    ++d->ref;
}

/*!
    Destroys the pen.
*/

QPen::~QPen()
{
    if (!--d->ref)
        delete d;
}

/*!
    \fn QPen::detach()
    Detaches from shared pen data to make sure that this pen is the
    only one referring the data.

    If multiple pens share common data, this pen dereferences the data
    and gets a copy of the data. Nothing is done if there is just a
    single reference.
*/

void QPen::detach_helper()
{
    QPenData *x = new QPenData;
    x->ref = 1;
    x->style = d->style;
    x->width = d->width;
    x->color = d->color;
    x->linest = d->linest;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        delete x;
}


/*!
    Assigns \a p to this pen and returns a reference to this pen.
*/

QPen &QPen::operator=(const QPen &p)
{
    qAtomicAssign(d, p.d);
    return *this;
}


/*!
    \fn Qt::PenStyle QPen::style() const

    Returns the pen style.

    \sa setStyle()
*/

/*!
    Sets the pen style to \a s.

    See the \l Qt::PenStyle documentation for a list of all the
    styles.

    \warning On Windows 95/98 and Macintosh, the style setting (other
    than \c Qt::NoPen and \c Qt::SolidLine) has no effect for lines with width
    greater than 1.

    \sa style()
*/

void QPen::setStyle(Qt::PenStyle s)
{
    if (d->style == s)
        return;
    detach();
    d->style = s;
    d->linest = (d->linest & ~Qt::MPenStyle) | s;
}


/*!
    \fn float QPen::width() const

    Returns the pen width with integer preceision.

    \sa setWidth()
*/


/*!
    \fn float QPen::widthF() const

    Returns the pen width with floating point precision.

    \sa setWidthF()
*/

/*!
    \fn QPen::setWidth(int width)

    \overload

    Sets the pen width to \a width
*/

/*!
    Sets the pen width to \a width.

    A line width of 0 will produce a 1 pixel wide line using a fast
    algorithm for diagonals. A line width of 1 will also produce a 1
    pixel wide line, but uses a slower more accurate algorithm for
    diagonals. For horizontal and vertical lines a line width of 0 is
    the same as a line width of 1. The cap and join style have no
    effect on 0-width lines.

    Setting a pen width with a negative value is not supported.

    \sa width()
*/

void QPen::setWidthF(float width)
{
    if (width < 0.f)
        qWarning("QPen::setWidthF(): Setting a pen width with a negative value is not defined.");
    if (qAbs(d->width - width) < 0.00000001f)
        return;
    detach();
    d->width = width;
}


/*!
    Returns the pen's cap style.

    \sa setCapStyle()
*/
Qt::PenCapStyle QPen::capStyle() const
{
    return (Qt::PenCapStyle)(d->linest & Qt::MPenCapStyle);
}

/*!
    Sets the pen's cap style to \a c.

    The default value is \c Qt::FlatCap. The cap style has no effect on
    0-width pens.

    \img pen-cap-styles.png Pen Cap Styles

    \warning On Windows 95/98 and Macintosh, the cap style setting has
    no effect. Wide lines are rendered as if the cap style was \c
    Qt::SquareCap.

    \sa capStyle()
*/

void QPen::setCapStyle(Qt::PenCapStyle c)
{
    if ((d->linest & Qt::MPenCapStyle) == c)
        return;
    detach();
    d->linest = (d->linest & ~Qt::MPenCapStyle) | c;
}

/*!
    Returns the pen's join style.

    \sa setJoinStyle()
*/
Qt::PenJoinStyle QPen::joinStyle() const
{
    return (Qt::PenJoinStyle)(d->linest & Qt::MPenJoinStyle);
}

/*!
    Sets the pen's join style to \a j.

    The default value is \c Qt::MiterJoin. The join style has no effect on
    0-width pens.

    \img pen-join-styles.png Pen Join Styles

    \warning On Windows 95/98 and Macintosh, the join style setting
    has no effect. Wide lines are rendered as if the join style was \c
    Qt::BevelJoin.

    \sa joinStyle()
*/

void QPen::setJoinStyle(Qt::PenJoinStyle j)
{
    if ((d->linest & Qt::MPenJoinStyle) == j)
        return;
    detach();
    d->linest = (d->linest & ~Qt::MPenJoinStyle) | j;
}

/*!
    \fn const QColor &QPen::color() const

    Returns the pen color.

    \sa setColor()
*/

/*!
    Sets the pen color to \a c.

    \sa color()
*/

void QPen::setColor(const QColor &c)
{
    detach();
    d->color = c;
}


/*!
    \fn bool QPen::operator!=(const QPen &p) const

    Returns true if the pen is different from \a p; otherwise returns
    false.

    Two pens are different if they have different styles, widths or
    colors.

    \sa operator==()
*/

/*!
    Returns true if the pen is equal to \a p; otherwise returns false.

    Two pens are equal if they have equal styles, widths and colors.

    \sa operator!=()
*/

bool QPen::operator==(const QPen &p) const
{
    return (p.d == d) || (p.d->linest == d->linest && p.d->width == d->width
                          && p.d->color == d->color);
}


/*!
    \fn bool QPen::isDetached()

    \internal
*/

/*****************************************************************************
  QPen stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QPen

    Writes the pen \a p to the stream \a s and returns a reference to
    the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPen &p)
{
    if (s.version() < 3)
        s << (quint8)p.style();
    else
        s << (quint8)(p.style() | p.capStyle() | p.joinStyle());
    if (s.version() < 7)
        s << (quint8)p.width();
    else {
        s << p.widthF();
    }
    return s << p.color();
}

/*!
    \relates QPen

    Reads a pen from the stream \a s into \a p and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QPen &p)
{
    quint8 style;
    quint8 width8 = 0;
    float width = 0;
    QColor color;
    s >> style;
    if (s.version() < 7)
        s >> width8;
    else
        s >> width;
    s >> color;
    p = QPen(color, (s.version() < 7 ? width8 : width), (Qt::PenStyle)style);
    return s;
}
#endif //QT_NO_DATASTREAM
