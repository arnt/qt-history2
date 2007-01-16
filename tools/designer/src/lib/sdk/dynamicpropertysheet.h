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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef DYNAMICPROPERTYSHEET_H
#define DYNAMICPROPERTYSHEET_H

#include <QtDesigner/extension.h>

QT_BEGIN_HEADER

class QDesignerDynamicPropertySheetExtension
{
public:
    virtual ~QDesignerDynamicPropertySheetExtension() {}

    virtual bool dynamicPropertiesAllowed() const = 0;
    virtual int addDynamicProperty(const QString &propertyName, const QVariant &value) = 0;
    virtual bool removeDynamicProperty(int index) = 0;
    virtual bool isDynamicProperty(int index) const = 0;
    virtual bool canAddDynamicProperty(const QString &propertyName) const = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerDynamicPropertySheetExtension, "com.trolltech.Qt.Designer.DynamicPropertySheet")

QT_END_HEADER

#endif // DYNAMICPROPERTYSHEET_H
