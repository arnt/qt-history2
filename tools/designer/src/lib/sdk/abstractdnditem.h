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

#ifndef ABSTRACTDNDITEM_H
#define ABSTRACTDNDITEM_H

#include "sdk_global.h"

class DomUI;
class QWidget;
class QPoint;

struct QT_SDK_EXPORT AbstractDnDItem
{
    enum DropType { MoveDrop, CopyDrop };

    virtual ~AbstractDnDItem() {}

    virtual DomUI *domUi() const = 0;
    virtual QWidget *widget() const = 0;
    virtual QWidget *decoration() const = 0;
    virtual QPoint hotSpot() const = 0;
    virtual DropType type() const = 0;
    virtual QWidget *source() const = 0;
};

#endif // ABSTRACTDNDITEM_H
