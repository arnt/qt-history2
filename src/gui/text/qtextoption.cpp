/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextoption.h"
#include "qapplication.h"
#include "qlist.h"

struct QTextOptionPrivate
{
    QList<qreal> tabStops;
};

/*!
    Constructs a text option with default properties for text.
*/
QTextOption::QTextOption()
    : align(Qt::AlignLeft),
      wordWrap(QTextOption::WordWrap),
      design(false),
      unused(0),
      f(0),
      tab(0),
      d(0)
{
    direction = QApplication::layoutDirection();
}

/*!
    Constructs a text option with the given \a alignment for text.
*/
QTextOption::QTextOption(Qt::Alignment alignment)
    : align(alignment),
      wordWrap(QTextOption::WordWrap),
      design(false),
      unused(0),
      f(0),
      tab(0),
      d(0)
{
    direction = QApplication::layoutDirection();
}

/*!
    Destroys the text option.
*/
QTextOption::~QTextOption()
{
    delete d;
}

/*!
    \fn QTextOption::QTextOption(const QTextOption &other)

    Construct a copy of the \a other text option.
*/
QTextOption::QTextOption(const QTextOption &o)
    : align(o.align),
      wordWrap(o.wordWrap),
      design(o.design),
      direction(o.direction),
      unused(o.unused),
      f(o.f),
      tab(o.tab),
      d(0)
{
    if (o.d)
        d = new QTextOptionPrivate(*o.d);
}

/*!
    \fn QTextOption &QTextOption::operator=(const QTextOption &other)

    Returns true if the text option is the same as the \a other text option;
    otherwise returns false.
*/
QTextOption &QTextOption::operator=(const QTextOption &o)
{
    if (this == &o)
        return *this;
    delete d; d = 0;
    align = o.align;
    wordWrap = o.wordWrap;
    design = o.design;
    direction = o.direction;
    unused = o.unused;
    f = o.f;
    tab = o.tab;
    if (o.d)
        d = new QTextOptionPrivate(*o.d);
    return *this;
}

/*!
    Sets the tab positions for the text layout to those specified by
    \a tabStops.

    \sa tabArray(), setTabStop()
*/
void QTextOption::setTabArray(QList<qreal> tabStops)
{
    if (!d)
        d = new QTextOptionPrivate;
    d->tabStops = tabStops;
}

/*!
    Returns a list of tab positions defined for the text layout.

    \sa setTabArray(), tabStop()
*/
QList<qreal> QTextOption::tabArray() const
{
    if (d)
        return d->tabStops;
    return QList<qreal>();
}

/*!
    \class QTextOption
    \brief The QTextOption class provides a description of general rich text
    properties.

    \ingroup text

    QTextOption is used to encapsulate common rich text properties in a single
    object. It contains information about text alignment, layout direction,
    word wrapping, and other standard properties associated with text rendering
    and layout.

    \sa QTextEdit, QTextDocument, QTextCursor
*/

/*!
    \enum QTextOption::WrapMode

    This enum describes how text is wrapped in a document.

    \value NoWrap       Text is not wrapped at all.
    \value WordWrap     Text is wrapped at word boundaries.
    \value ManualWrap   Wrapping occurs at a manually specified length from
                        the start of the line.
    \value WrapAnywhere Text can be wrapped at any point on a line, even if
                        it occurs in the middle of a word.
    \value WrapAtWordBoundaryOrAnywhere If possible, wrapping occurs at a word
                        boundary; otherwise it will occur at the appropriate
                        point on the line, even in the middle of a word.
*/

/*!
  \fn void QTextOption::setUseDesignMetrics(bool enable)

    If \a enable is true then the layout will use design metrics;
    otherwise it will use the metrics of the paint device (which is
    the default behavior).

    \sa useDesignMetrics()
*/

/*!
  \fn bool QTextOption::useDesignMetrics() const

    Returns true if the layout uses design rather than device metrics;
    otherwise returns false.

    \sa setUseDesignMetrics()
*/

/*!
  \fn Qt::Alignment QTextOption::alignment() const

  Returns the text alignment defined by the option.

  \sa setAlignment()
*/

/*!
  \fn void QTextOption::setAlignment(Qt::Alignment alignment);

  Sets the option's text alignment to the specified \a alignment.

  \sa alignment()
*/

/*!
  \fn Qt::LayoutDirection QTextOption::textDirection() const

  Returns the direction of the text layout defined by the option.

  \sa setTextDirection()
*/

/*!
  \fn void QTextOption::setTextDirection(Qt::LayoutDirection direction)

  Sets the direction of the text layout defined by the option to the
  given \a direction.

  \sa textDirection()
*/

/*!
  \fn WrapMode QTextOption::wrapMode() const

  Returns the text wrap mode defined by the option.

  \sa setWrapMode()
*/

/*!
  \fn void QTextOption::setWrapMode(WrapMode mode)

  Sets the option's text wrap mode to the given \a mode.
*/

/*!
  \enum QTextOption::Flag

  \value IncludeTrailingSpaces  
*/

/*!
  \fn Flags QTextOption::flags() const

  Returns the flags associated with the option.

  \sa setFlags()
*/

/*!
  \fn void QTextOption::setFlags(Flags flags)

  Sets the flags associated with the option to the given \a flags.

  \sa flags()
*/

/*!
  \fn qreal QTextOption::tabStop() const

  Returns the distance in device units between tab stops.

  \sa setTabStop(), tabArray()
*/

/*!
  \fn void QTextOption::setTabStop(qreal tabStop)

  Sets the distance in device units between tab stops to the value specified
  by \a tabStop.

  \sa tabStop(), setTabArray()
*/
