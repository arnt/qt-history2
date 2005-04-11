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

#ifndef QLAYOUTWIDGET_PROPERTYSHEET_H
#define QLAYOUTWIDGET_PROPERTYSHEET_H

#include <qdesigner_propertysheet.h>

class QLayoutWidget;

class QLayoutWidgetPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    QLayoutWidgetPropertySheet(QLayoutWidget *object, QObject *parent = 0);
    virtual ~QLayoutWidgetPropertySheet();

    virtual void setProperty(int index, const QVariant &value);
};

class QLayoutWidgetPropertySheetFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    QLayoutWidgetPropertySheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // QLAYOUTWIDGET_PROPERTYSHEET_H
