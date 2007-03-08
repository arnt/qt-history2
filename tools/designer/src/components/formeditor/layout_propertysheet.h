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

#ifndef LAYOUT_PROPERTYSHEET_H
#define LAYOUT_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>

class QLayout;

namespace qdesigner_internal {

class LayoutPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit LayoutPropertySheet(QLayout *object, QObject *parent = 0);
    virtual ~LayoutPropertySheet();

    virtual void setProperty(int index, const QVariant &value);
    virtual QVariant property(int index) const;
    virtual bool reset(int index);
    void setChanged(int index, bool changed);
private:
    QLayout *m_layout;
};

class LayoutPropertySheetFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    explicit LayoutPropertySheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal

#endif // LAYOUT_PROPERTYSHEET_H
