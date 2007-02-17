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

#ifndef QICONENGINEPLUGIN_H
#define QICONENGINEPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QIconEngine;
class QIconEngineV2;

struct Q_GUI_EXPORT QIconEngineFactoryInterface : public QFactoryInterface
{
    virtual QIconEngine *create(const QString &filename) = 0;
};

#define QIconEngineFactoryInterface_iid "com.trolltech.Qt.QIconEngineFactoryInterface"
Q_DECLARE_INTERFACE(QIconEngineFactoryInterface, QIconEngineFactoryInterface_iid)

class Q_GUI_EXPORT QIconEnginePlugin : public QObject, public QIconEngineFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QIconEngineFactoryInterface:QFactoryInterface)
public:
    QIconEnginePlugin(QObject *parent = 0);
    ~QIconEnginePlugin();

    virtual QStringList keys() const = 0;
    virtual QIconEngine *create(const QString &filename) = 0;
};

// ### Qt 5: remove version 2
struct Q_GUI_EXPORT QIconEngineFactoryInterfaceV2 : public QFactoryInterface
{
    virtual QIconEngineV2 *create(const QString &filename = QString()) = 0;
};

#define QIconEngineFactoryInterfaceV2_iid "com.trolltech.Qt.QIconEngineFactoryInterfaceV2"
Q_DECLARE_INTERFACE(QIconEngineFactoryInterfaceV2, QIconEngineFactoryInterfaceV2_iid)

class Q_GUI_EXPORT QIconEnginePluginV2 : public QObject, public QIconEngineFactoryInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(QIconEngineFactoryInterfaceV2:QFactoryInterface)
public:
    QIconEnginePluginV2(QObject *parent = 0);
    ~QIconEnginePluginV2();

    virtual QStringList keys() const = 0;
    virtual QIconEngineV2 *create(const QString &filename = QString()) = 0;
};

QT_END_HEADER

#endif // QICONENGINEPLUGIN_H
