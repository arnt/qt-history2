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

#ifndef QACCESSIBLEBRIDGE_H
#define QACCESSIBLEBRIDGE_H

#ifndef QT_H
#include "qplugin.h"
#include "qfactoryinterface.h"
#endif // QT_H

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleInterface;

class QAccessibleBridge
{
public:
    virtual void setRootObject(QAccessibleInterface *) = 0;
    virtual void notifyAccessibilityUpdate(int, QAccessibleInterface*, int) = 0;
};

struct Q_GUI_EXPORT QAccessibleBridgeFactoryInterface : public QFactoryInterface
{
    virtual QAccessibleBridge* create(const QString& name) = 0;
};

Q_DECLARE_INTERFACE(QAccessibleBridgeFactoryInterface, "http://trolltech.com/Qt/QAccessibleBridgeFactoryInterface")

class Q_GUI_EXPORT QAccessibleBridgePlugin : public QObject, public QAccessibleBridgeFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QAccessibleBridgeFactoryInterface:QFactoryInterface)
public:
    QAccessibleBridgePlugin(QObject *parent = 0);
    ~QAccessibleBridgePlugin();

    virtual QStringList keys() const = 0;
    virtual QAccessibleBridge *create(const QString &key) = 0;
};

#endif // QT_NO_SQL

#endif // QSQLDRIVERPLUGIN_H
