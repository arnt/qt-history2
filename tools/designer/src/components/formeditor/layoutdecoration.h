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

#ifndef LAYOUTDECORATION_H
#define LAYOUTDECORATION_H

#include <extension.h>
#include <QObject>

class QPoint;

struct ILayoutDecoration
{
    virtual ~ILayoutDecoration() {}
    
    virtual int findItemAt(const QPoint &pos) const = 0;
    virtual void adjustIndicator(const QPoint &pos, int index) = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(ILayoutDecoration, "http://trolltech.com/Qt/IDE/LayoutDecoration")

#endif // LAYOUTDECORATION_H
