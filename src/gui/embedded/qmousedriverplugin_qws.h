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

#ifndef QMOUSEDRIVERPLUGIN_QWS_H
#define QMOUSEDRIVERPLUGIN_QWS_H

#include "QtCore/qplugin.h"
#include "QtCore/qfactoryinterface.h"

#ifndef QT_NO_COMPONENT

class QWSMouseHandler;

struct Q_GUI_EXPORT QWSMouseHandlerFactoryInterface : public QFactoryInterface
{
    virtual QWSMouseHandler* create(const QString& name) = 0;
};

Q_DECLARE_INTERFACE(QWSMouseHandlerFactoryInterface, "http://trolltech.com/Qt/QWSMouseHandlerFactoryInterface")


class Q_GUI_EXPORT QMouseDriverPlugin : public QObject, public QWSMouseHandlerFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QWSMouseHandlerFactoryInterface:QFactoryInterface)
public:
    explicit QMouseDriverPlugin(QObject *parent = 0);
    ~QMouseDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QWSMouseHandler* create(const QString& driver, const QString &device) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QMOUSEDRIVERPLUGIN_QWS_H
