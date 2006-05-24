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

#include "multipagewidgetextensionfactory.h"
#include "multipagewidgetcontainerextension.h"
#include "multipagewidget.h"

MultiPageWidgetExtensionFactory::MultiPageWidgetExtensionFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{}

QObject *MultiPageWidgetExtensionFactory::createExtension(QObject *object,
                                                          const QString &iid,
                                                          QObject *parent) const
{
    MultiPageWidget *widget = qobject_cast<MultiPageWidget*>(object);

    if (widget && (iid == Q_TYPEID(QDesignerContainerExtension))) {
        return new MultiPageWidgetContainerExtension(widget, parent);
    } else {
        return 0;
    }
}
