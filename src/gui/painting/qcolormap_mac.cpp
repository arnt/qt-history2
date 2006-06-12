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

#include "qcolormap.h"
#include "qcolor.h"


class QColormapPrivate
{
public:
    inline QColormapPrivate()
        : ref(1)
    { }

    QAtomic ref;
};
static QColormap *qt_mac_global_map = 0;

void QColormap::initialize()
{
    qt_mac_global_map = new QColormap;
}

void QColormap::cleanup()
{
    delete qt_mac_global_map;
    qt_mac_global_map = 0;
}

QColormap QColormap::instance(int)
{
    return *qt_mac_global_map;
}

QColormap::QColormap() : d(new QColormapPrivate)
{}

QColormap::QColormap(const QColormap &colormap) :d (colormap.d)
{ d->ref.ref(); }

QColormap::~QColormap()
{
    if (!d->ref.deref())
        delete d;
}

QColormap::Mode QColormap::mode() const
{ return QColormap::Direct; }

int QColormap::depth() const
{
    return 32;
}

int QColormap::size() const
{
    return -1;
}

uint QColormap::pixel(const QColor &color) const
{ return color.rgba(); }

const QColor QColormap::colorAt(uint pixel) const
{ return QColor(pixel); }

const QVector<QColor> QColormap::colormap() const
{ return QVector<QColor>(); }

QColormap &QColormap::operator=(const QColormap &colormap)
{ qAtomicAssign(d, colormap.d); return *this; }
