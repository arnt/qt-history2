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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QtDesigner/sdk_global.h>
#include <QtDesigner/extension.h>
#include <QtCore/QVariant>

QT_BEGIN_HEADER

class QDESIGNER_SDK_EXPORT QDesignerScriptExtension
{
public:
    virtual ~QDesignerScriptExtension();

    virtual QVariantMap data() const = 0;
    virtual void setData(const QVariantMap &) = 0;

    virtual QString script() const = 0;

};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerScriptExtension, "com.trolltech.Qt.Designer.Script")

QT_END_HEADER

#endif // SCRIPT_H
