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

#include "imagecollection.h"

ImageCollection::ImageCollection(AbstractFormEditor *core, QObject *parent)
    : AbstractImageCollection(parent), m_core(core)
{
}

ImageCollection::~ImageCollection()
{
}

AbstractFormEditor *ImageCollection::core() const
{
    return m_core;
}

QString ImageCollection::fileName() const
{
    return QString::null;
}

QString ImageCollection::prefix() const
{
    return QString::null;
}

int ImageCollection::count() const
{
    return 0;
}

QString ImageCollection::item(int index) const
{
    Q_UNUSED(index);
    return QString::null;
}
