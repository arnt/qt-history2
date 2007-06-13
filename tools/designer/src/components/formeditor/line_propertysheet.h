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

#ifndef LINE_PROPERTYSHEET_H
#define LINE_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>
#include <qdesigner_widget_p.h>
#include <extensionfactory_p.h>

namespace qdesigner_internal {

class LinePropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit LinePropertySheet(Line *object, QObject *parent = 0);
    virtual ~LinePropertySheet();

    virtual void setProperty(int index, const QVariant &value);
    virtual bool isVisible(int index) const;
    virtual QString propertyGroup(int index) const;
};

typedef ExtensionFactory<QDesignerPropertySheetExtension, Line, LinePropertySheet> LinePropertySheetFactory;
}  // namespace qdesigner_internal

#endif // LINE_PROPERTYSHEET_H
