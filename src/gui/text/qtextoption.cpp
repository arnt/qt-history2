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

#include <qtextoption.h>

#include <QtCore/qlist.h>

struct QTextOptionPrivate
{
    QList<qReal> tabStops;
};

QTextOption::QTextOption()
    : align(Qt::AlignLeft),
      wordWrap(QTextOption::WordWrap),
      direction(Qt::LeftToRight),
      design(false),
      unused(0),
      f(0),
      tab(0),
      d(0)
{
}

QTextOption::QTextOption(Qt::Alignment alignment)
    : align(alignment),
      wordWrap(QTextOption::WordWrap),
      direction(Qt::LeftToRight),
      design(false),
      unused(0),
      f(0),
      tab(0),
      d(0)
{
}

QTextOption::~QTextOption()
{
    delete d;
}

QTextOption::QTextOption(const QTextOption &o)
    : align(o.align),
      wordWrap(o.wordWrap),
      direction(o.direction),
      design(o.design),
      unused(o.unused),
      f(o.f),
      tab(o.tab)
{
    if (o.d)
        d = new QTextOptionPrivate(*o.d);
}

QTextOption &QTextOption::operator=(const QTextOption &o)
{
    if (this == &o)
        return *this;
    delete d; d = 0;
    align = o.align;
    wordWrap = o.wordWrap;
    direction =o.direction;
    design = o.design;
    unused = o.unused;
    f = o.f;
    tab = o.tab;
    if (o.d)
        d = new QTextOptionPrivate(*o.d);
    return *this;
}

void QTextOption::setTabArray(QList<qReal> tabStops)
{
    if (!d)
        d = new QTextOptionPrivate;
    d->tabStops = tabStops;
}

QList<qReal> QTextOption::tabArray() const
{
    if (d)
        return d->tabStops;
    return QList<qReal>();
}


/*!
  \fn void QTextOption::useDesignMetrics(bool b)

    If \a b is true then the layouting will use design metrics;
    otherwise it will use the metrics of the paint device (which is
    the default behavior).

    \sa usesDesignMetrics()
*/

/*!
  \fn bool QTextOption::usesDesignMetrics() const

    Returns true if this layouting uses design rather than device
    metrics; otherwise returns false.

    \sa useDesignMetrics()
*/
