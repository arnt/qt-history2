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

#include "qstylepainter.h"

/*!
    \class QStylePainter

    \brief The QStylePainter class is a convenience class for drawing QStyle
    elements inside a widget.

    \ingroup appearance multimedia

    QStylePainter is a QPainter that also has a higher-level interface to
    QStyle's draw* functions. QStylePainter can only be used on a widget as it
    uses the widget's underlying style to call the style functions.

    Here is a standard use case for using QStylePainter

    \code

    void MyWidget::paintEvent(QPaintEvent *)
    {
        QStylePainter painter(this);

        painter.drawControl(QStyle::CE_PushButton, getStyleOption());

        // Further painting with the painter.
        ...
    }

*/

/*!
    \fn QStylePainter::QStylePainter()

    Constructs a QStylePainter.
*/

/*!
    \fn QStylePainter::QStylePainter(QWidget *widget)

    Construct a QStylePainter using widget \a widget for its paint device.
*/

/*!
    \fn QStylePainter::QStylePainter(QPaintDevice *pd,  Widget *widget)

    Construct a QStylePainter using \a pd for its paint device, and
    attributes from \a widget.
*/


/*!
    \fn bool QStylePainter::begin(QWidget *widget)

    Begin painting operations on widget \a widget.
    This is automatically called by the constructor that takes a QWidget.
*/

/*! \overload
    \fn bool QStylePainter::begin(QPaintDevice *pd QWidget *widget)

    Begin painting operations on paint device \a pd as if it was \a
    widget.  This is automatically called by the constructor that
    takes a QPaintDevice and a QWidget.
*/


/*!
    \fn void QStylePainter::drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption &option)

    Use the widget's style to draw a primitive element \a pe specified by QStyleOption \a option.

    \sa QStyle::drawPrimitive
*/

/*!
    \fn void QStylePainter::drawControl(QStyle::ControlElement ce, const QStyleOption &option)

    Use the widget's style to draw a control element \a ce specified by QStyleOption \a option.

    \sa QStyle::drawControl
*/

/*!
    void QStylePainter::drawControlMask(QStyle::ControlElement element, const QStyleOption &option)

    Use the widget's style to draw a control element's mask \a ce specified by
    QStyleOption \a option.

    \sa QStyle::drawControlMask
*/

/*!
  \fn void QStylePainter::drawComplexControl(QStyle::ComplexControl cc,
                                             const QStyleOptionComplex &option)

    Use the widget's style to draw a complex control \a cc specified by the
    QStyleOptionComplex \a option.

    \sa QStyle::drawComplexControl
*/


/*!
    \fn void QStylePainter::drawComplexControlMask(QStyle::ComplexControl cc,
                                                   const QStyleOptionComplex &option);

    Use the widget's style to draw a complex control element's mask \a cc
    specified by QStyleOptionComplex \a option.

    \sa QStyle::drawComplexControlMask
*/

/*!
    \fn void QStylePainter::drawItem(const QRect &rect, int flags, const QPalette &pal,
                                     bool enabled, const QString &text, int len = -1,
                                     const QColor *penColor = 0)

    Draws the \a text in rectangle \a rect and palette \a pal.

    The pen color is specified with \a penColor. The
    \a enabled bool indicates whether or not the item is enabled;
    when reimplementing this bool should influence how the item is
    drawn.

    If \a len is -1 (the default), all the \a text is drawn;
    otherwise only the first \a len characters of \a text are drawn.
    The text is aligned and wrapped according to \a
    alignment.

    \sa Qt::Alignment QStyle::drawItem
*/


/*!
    \fn void QStylePainter::drawItem(const QRect &rect, int flags, const QPalette &pal,
                                     bool enabled, const QPixmap &pixmap,
                                     const QColor *penColor = 0)

    \overload

    Draws the \a pixmap in rectangle \a rect using \a painter and the
    palette \a pal.

    \sa QStyle::drawItem
*/


/*!
    \fn void QStylePainter::drawItem(const QRect &rect, int flags, const QPalette &pal,
                                     bool enabled, const QPixmap &pixmap, const QString &text,
                                     int len = -1, const QColor *penColor = 0)

    \overload
    Draws the \a pixmap or \a text in rectangle \a rect using \a painter and
    palette \a pal.

    \sa QStyle::drawItem
*/

/*!
    \fn QStyle *QStylePainter::style() const

    Return the current style used by the QStylePainter.
*/
