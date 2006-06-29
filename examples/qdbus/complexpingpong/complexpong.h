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

#ifndef COMPLEXPONG_H
#define COMPLEXPONG_H

#include <QtCore/QObject>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusVariant>

class Pong: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.trolltech.QtDBus.ComplexPong.Pong")
    Q_PROPERTY(QString value READ value WRITE setValue)
public:
    QString m_value;
    QString value() const;
    void setValue(const QString &newValue);

    Pong(QObject *obj) : QDBusAbstractAdaptor(obj)
    { }
signals:
    void aboutToQuit();
public slots:
    QDBusVariant query(const QString &query);
    Q_NOREPLY void quit();
};

#endif
