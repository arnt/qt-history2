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

#include "spacer_propertysheet.h"
#include "qdesigner_widget.h"
#include "formwindow.h"
#include "spacer.h"

#include <qextensionmanager.h>

#include <QLayout>
#include <QMetaObject>
#include <QMetaProperty>
#include <qdebug.h>

SpacerPropertySheet::SpacerPropertySheet(Spacer *object, QObject *parent)
    : QDesignerPropertySheet(object, parent)
{
    m_fakeProperties.clear();
    
    for (int i=0; i<count(); ++i)
        setVisible(i, false);

    setVisible(indexOf("orientation"), true);
    setVisible(indexOf("sizeType"), true);
    setVisible(indexOf("sizeHint"), true);
}

SpacerPropertySheet::~SpacerPropertySheet()
{
}

void SpacerPropertySheet::setProperty(int index, const QVariant &value)
{
    QDesignerPropertySheet::setProperty(index, value);
}


SpacerPropertySheetFactory::SpacerPropertySheetFactory(QExtensionManager *parent)
    : DefaultExtensionFactory(parent)
{
}

QObject *SpacerPropertySheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(IPropertySheet))
        return 0;

    if (Spacer *o = qt_cast<Spacer*>(object))
        return new SpacerPropertySheet(o, parent);

    return 0;
}
