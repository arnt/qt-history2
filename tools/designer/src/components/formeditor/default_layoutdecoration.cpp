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

#include "default_layoutdecoration.h"
#include "qlayout_widget_p.h"

#include <QtDesigner/QDesignerMetaDataBaseItemInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>

namespace qdesigner_internal {

// ---- QDesignerLayoutDecorationFactory ----
QDesignerLayoutDecorationFactory::QDesignerLayoutDecorationFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerLayoutDecorationFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (!object->isWidgetType() || iid != Q_TYPEID(QDesignerLayoutDecorationExtension))
        return 0;

    QDesignerFormWindowInterface *fw = 0;
    QWidget *widget = qobject_cast<QWidget*>(object);
    bool match = false;
    do {
        if (const QLayoutWidget *layoutWidget = qobject_cast<const QLayoutWidget*>(widget)) {
            fw = layoutWidget->formWindow();
            match = true;
            break;
        }

        QLayout *layout = widget->layout();
        if (!layout)
            break;

        fw = QDesignerFormWindowInterface::findFormWindow(widget);
        if (!fw)
            break;

        if (fw->core()->metaDataBase()->item(layout))
            match = true;
    } while (false);
    if (match)
        return QLayoutSupport::createLayoutSupport(fw, widget, parent);
    return 0;
}
}
