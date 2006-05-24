/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MULTIPAGEWIDGETEXTENSIONFACTORY_H
#define MULTIPAGEWIDGETEXTENSIONFACTORY_H

#include <QtDesigner/QExtensionFactory>

class QExtensionManager;

class MultiPageWidgetExtensionFactory: public QExtensionFactory
{
    Q_OBJECT

public:
    MultiPageWidgetExtensionFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif
