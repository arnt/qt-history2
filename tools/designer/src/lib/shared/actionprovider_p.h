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

#ifndef ACTIONPROVIDER_H
#define ACTIONPROVIDER_H

#include <QtDesigner/extension.h>

class QAction;
class QRect;
class QPoint;

class QDesignerActionProviderExtension
{
public:
    virtual ~QDesignerActionProviderExtension() {}

    virtual QRect actionGeometry(QAction *action) const = 0;
    virtual QAction *actionAt(const QPoint &pos) const = 0;

    virtual void adjustIndicator(const QPoint &pos) = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerActionProviderExtension, "com.trolltech.Qt.Designer.ActionProvider")


#endif // ACTIONPROVIDER_H
