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

#include "qcolormap.h"
#include "qcolor.h"

class QColormapPrivate
{
public:
    QColormapPrivate()
        : mode(QColormap::Direct), depth(0)
    { ref = 0; }
    ~QColormapPrivate()
    {}

    QAtomic ref;

    QColormap::Mode mode;
    int depth;
};


void QColormap::initialize()
{
}

void QColormap::cleanup()
{
}

QColormap QColormap::instance(int screen)
{
    return QColormap();
}

QColormap::QColormap()
    : d(new QColormapPrivate)
{ d->ref = 1; }

QColormap::QColormap(const QColormap &colormap)
    :d (colormap.d)
{ ++d->ref; }

QColormap::~QColormap()
{
    if (!--d->ref)
        delete d;
}

QColormap::Mode QColormap::mode() const
{ return d->mode; }

int QColormap::depth() const
{ return d->depth; }

int QColormap::size() const
{
    return 0;
}

uint QColormap::pixel(const QColor &color) const
{
    return 0;
}

QColor QColormap::colorAt(uint pixel) const
{
    return QColor();
}
