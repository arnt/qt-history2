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

#ifndef QKBDDRIVERPLUGIN_QWS_H
#define QKBDDRIVERPLUGIN_QWS_H

#ifndef QT_H
#include "qplugin.h"
#include "qfactoryinterface.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

class QWSKeyboardHandler;

struct Q_GUI_EXPORT QWSKeyboardHandlerFactoryInterface : public QFactoryInterface
{
    virtual QWSKeyboardHandler* create(const QString& name) = 0;
};

Q_DECLARE_INTERFACE(QWSKeyboardHandlerFactoryInterface, "http://trolltech.com/Qt/QWSKeyboardHandlerFactoryInterface")

class Q_GUI_EXPORT QKbdDriverPlugin : public QObject, public QWSKeyboardHandlerFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QWSKeyboardHandlerFactoryInterface:QFactoryInterface)
public:
    QKbdDriverPlugin(QObject *parent = 0);
    ~QKbdDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QWSKeyboardHandler* create(const QString& driver, const QString &device) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QKBDDRIVERPLUGIN_QWS_H
