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

#include "qlayoutwidget_propertysheet.h"
#include "qlayout_widget_p.h"
#include "formwindow.h"
#include "formeditor.h"

#include <QtDesigner/QExtensionManager>

#include <QLayout>

using namespace qdesigner_internal;

QLayoutWidgetPropertySheet::QLayoutWidgetPropertySheet(QLayoutWidget *object, QObject *parent)
    : QDesignerPropertySheet(object, parent)
{
    m_fakeProperties.clear();
}

QLayoutWidgetPropertySheet::~QLayoutWidgetPropertySheet()
{
}

bool QLayoutWidgetPropertySheet::isVisible(int index) const
{
    if (propertyGroup(index) == QLatin1String("Layout"))
        return QDesignerPropertySheet::isVisible(index);
    return false;
}

void QLayoutWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
    QDesignerPropertySheet::setProperty(index, value);
}

bool QLayoutWidgetPropertySheet::dynamicPropertiesAllowed() const
{
    return false;
}

QLayoutWidgetPropertySheetFactory::QLayoutWidgetPropertySheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QLayoutWidgetPropertySheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerPropertySheetExtension))
        return 0;

    if (QLayoutWidget *o = qobject_cast<QLayoutWidget*>(object))
        return new QLayoutWidgetPropertySheet(o, parent);

    return 0;
}
