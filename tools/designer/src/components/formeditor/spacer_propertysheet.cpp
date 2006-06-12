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

#include "spacer_propertysheet.h"
#include "qdesigner_widget_p.h"
#include "formwindow.h"
#include "spacer_widget_p.h"

#include <QtDesigner/QExtensionManager>

#include <QtGui/QLayout>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

SpacerPropertySheet::SpacerPropertySheet(Spacer *object, QObject *parent)
    : QDesignerPropertySheet(object, parent)
{
    m_fakeProperties.clear();
}

SpacerPropertySheet::~SpacerPropertySheet()
{
}

bool SpacerPropertySheet::isVisible(int index) const
{
    QString group = propertyGroup(index);

    return group == QLatin1String("Spacer");
}

void SpacerPropertySheet::setProperty(int index, const QVariant &value)
{
    QDesignerPropertySheet::setProperty(index, value);
}


SpacerPropertySheetFactory::SpacerPropertySheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *SpacerPropertySheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerPropertySheetExtension))
        return 0;

    if (Spacer *o = qobject_cast<Spacer*>(object))
        return new SpacerPropertySheet(o, parent);

    return 0;
}
