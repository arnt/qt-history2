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

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtDBus/QtDBus>

void method1()
{
    qDebug() << "Method 1:";

    QDBusReply<QStringList> reply = QDBus::sessionBus().interface()->registeredServiceNames();
    if (!reply.isValid()) {
        qDebug() << "Error:" << reply.error().message();
        exit(1);
    }
    foreach (QString name, reply.value())
        qDebug() << name;
}

void method2()
{
    qDebug() << "Method 2:";

    QDBusConnection bus = QDBus::sessionBus();
    QDBusInterface dbus_iface("org.freedesktop.DBus", "/org/freedesktop/DBus",
                              "org.freedesktop.DBus", bus);
    qDebug() << dbus_iface.call("ListNames").at(0);
}

void method3()
{
    qDebug() << "Method 3:";
    qDebug() << QDBus::sessionBus().interface()->registeredServiceNames().value();
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    method1();
    method2();
    method3();

    return 0;
}
