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

#include "qlayoutwidget_propertysheet.h"
#include "qdesigner_widget.h"
#include "formwindow.h"
#include "formeditor.h"

#include <qextensionmanager.h>

#include <QLayout>
#include <QMetaObject>
#include <QMetaProperty>
#include <qdebug.h>

QLayoutWidgetPropertySheet::QLayoutWidgetPropertySheet(QLayoutWidget *object, QObject *parent)
    : QDesignerPropertySheet(object, parent)
{
    m_fakeProperties.clear();
    
    for (int index = 0; index < count(); ++index) {
        QString pname = propertyName(index);
        setVisible(index, false);
        if (pname == QLatin1String("margin")
                || pname == QLatin1String("spacing")
                /*|| pname == QLatin1String("objectName") ### */ )
            setVisible(index, true);
    }
}

QLayoutWidgetPropertySheet::~QLayoutWidgetPropertySheet()
{
}

void QLayoutWidgetPropertySheet::setProperty(int index, const QVariant &value)
{    
    QDesignerPropertySheet::setProperty(index, value);
    
    QLayoutWidget *l = static_cast<QLayoutWidget*>(m_object);
    AbstractFormEditor *core = l->formWindow()->core();
    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(core->extensionManager(), l->layout())) {
        sheet->setChanged(sheet->indexOf(propertyName(index)), true);
    }
}


QLayoutWidgetPropertySheetFactory::QLayoutWidgetPropertySheetFactory(QExtensionManager *parent)
    : DefaultExtensionFactory(parent)
{
}

QObject *QLayoutWidgetPropertySheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(IPropertySheet))
        return 0;

    if (QLayoutWidget *o = qobject_cast<QLayoutWidget*>(object))
        return new QLayoutWidgetPropertySheet(o, parent);

    return 0;
}
