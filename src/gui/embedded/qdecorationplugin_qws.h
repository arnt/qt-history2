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

#ifndef QDECORATIONPLUGIN_H
#define QDECORATIONPLUGIN_H

#include "QtCore/qplugin.h"
#include "QtCore/qfactoryinterface.h"

class QDecoration;

struct QDecorationFactoryInterface : public QFactoryInterface
{
    virtual QDecoration *create(const QString &key) = 0;
};

Q_DECLARE_INTERFACE(QDecorationFactoryInterface,  "http://trolltech.com/Qt/QDecorationFactoryInterface")

class QDecorationPlugin : public QObject,  public QDecorationFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QDecorationFactoryInterface:QFactoryInterface)
        public:
    QDecorationPlugin(QObject *parent = 0);
    ~QDecorationPlugin();

    virtual QStringList keys() const = 0;
    virtual QDecoration *create(const QString &key) = 0;
};

#endif // QDECORATIONPLUGIN_H
