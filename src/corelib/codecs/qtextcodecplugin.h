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

#ifndef QTEXTCODECPLUGIN_H
#define QTEXTCODECPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>
#include <QtCore/qlist.h>
#include <QtCore/qbytearray.h>

QT_MODULE(Core)

#ifndef QT_NO_TEXTCODECPLUGIN

class QTextCodec;

struct Q_CORE_EXPORT QTextCodecFactoryInterface : public QFactoryInterface
{
    virtual QTextCodec *create(const QString &key) = 0;
};

#define QTextCodecFactoryInterface_iid "com.trolltech.Qt.QTextCodecFactoryInterface"

Q_DECLARE_INTERFACE(QTextCodecFactoryInterface, QTextCodecFactoryInterface_iid)

class Q_CORE_EXPORT QTextCodecPlugin : public QObject, public QTextCodecFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextCodecFactoryInterface:QFactoryInterface)
public:
    explicit QTextCodecPlugin(QObject *parent = 0);
    ~QTextCodecPlugin();

    virtual QList<QByteArray> names() const = 0;
    virtual QList<QByteArray> aliases() const = 0;
    virtual QTextCodec *createForName(const QByteArray &name) = 0;

    virtual QList<int> mibEnums() const = 0;
    virtual QTextCodec *createForMib(int mib) = 0;

private:
    QStringList keys() const;
    QTextCodec *create(const QString &name);
};

#endif // QT_NO_TEXTCODECPLUGIN

#endif // QTEXTCODECPLUGIN_H
