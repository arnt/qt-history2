/****************************************************************************
**
** Definition of the QAccessiblePlugin class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEPLUGIN_H
#define QACCESSIBLEPLUGIN_H

#ifndef QT_H
#include "qaccessible.h"
#include "qfactoryinterface.h"
#endif // QT_H

#ifndef QT_NO_ACCESSIBILITY

class QStringList;
class QAccessibleInterface;

struct Q_GUI_EXPORT QAccessibleFactoryInterface : public QAccessible, public QFactoryInterface
{
    virtual QAccessibleInterface* create(const QString &key, QObject *object) = 0;
};

Q_DECLARE_INTERFACE(QAccessibleFactoryInterface, "http://trolltech.com/Qt/QAccessibleFactoryInterface")

class QAccessiblePluginPrivate;

class Q_GUI_EXPORT QAccessiblePlugin : public QObject, public QAccessibleFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QAccessibleFactoryInterface:QFactoryInterface)
public:
    QAccessiblePlugin(QObject *parent = 0);
    ~QAccessiblePlugin();

    virtual QStringList keys() const = 0;
    virtual QAccessibleInterface *create(const QString &key, QObject *object) = 0;
};

#endif // QT_NO_ACCESSIBILITY

#endif // QACCESSIBLEPLUGIN_H
