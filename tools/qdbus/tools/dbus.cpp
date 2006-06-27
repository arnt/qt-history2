/****************************************************************************
  **
  ** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
  **
  ** This file is part of the $MODULE$ of the Qt Toolkit.
  **
  ** $TROLLTECH_DUAL_LICENSE$
  **
  ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
  ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
  **
  ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/qmetaobject.h>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtDBus>
#include <private/qdbusutil_p.h>

QDBusConnection connection("");

void listObjects(const QString &service, const QString &path)
{
    QDBusInterface iface(service, path.isEmpty() ? "/" : path,
                         "org.freedesktop.DBus.Introspectable", connection);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "Cannot introspect object %s at %s:\n%s (%s)\n",
                qPrintable(path.isEmpty() ? "/" : path), qPrintable(service),
                qPrintable(err.name()), qPrintable(err.message()));
        exit(1);
    }
    QDBusReply<QString> xml = iface.call("Introspect");

    if (!xml.isValid())
        return;                 // silently

    QDomDocument doc;
    doc.setContent(xml);
    QDomElement node = doc.documentElement();
    QDomElement child = node.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == QLatin1String("node")) {
            QString sub = path + '/' + child.attribute("name");
            printf("%s\n", qPrintable(sub));
            listObjects(service, sub);
        }
        child = child.nextSiblingElement();
    }
}

void listInterface(const QString &service, const QString &path, const QString &interface)
{
    QDBusInterface iface(service, path, interface, connection);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "Interface '%s' not available in object %s at %s:\n%s (%s)\n",
                qPrintable(interface), qPrintable(path), qPrintable(service),
                qPrintable(err.name()), qPrintable(err.message()));
        exit(1);
    }
    const QMetaObject *mo = iface.metaObject();

    // properties
    for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        QMetaProperty mp = mo->property(i);
        printf("property ");

        if (mp.isReadable() && mp.isWritable())
            printf("readwrite");
        else if (mp.isReadable())
            printf("read");
        else
            printf("write");

        printf(" %s %s.%s\n", mp.typeName(), qPrintable(interface), mp.name());
    }

    // methods (signals and slots)
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);

        QByteArray signature = mm.signature();
        signature.truncate(signature.indexOf('('));
        printf("%s %s%s%s %s.%s(",
               mm.methodType() == QMetaMethod::Signal ? "signal" : "method",
               mm.tag(), *mm.tag() ? " " : "",
               *mm.typeName() ? mm.typeName() : "void",
               qPrintable(interface), signature.constData());

        QList<QByteArray> types = mm.parameterTypes();
        QList<QByteArray> names = mm.parameterNames();
        bool first = true;
        for (int i = 0; i < types.count(); ++i) {
            printf("%s%s",
                   first ? "" : ", ",
                   types.at(i).constData());
            if (!names.at(i).isEmpty())
                printf(" %s", names.at(i).constData());
            first = false;
        }
        printf(")\n");
    }
}

void listAllInterfaces(const QString &service, const QString &path)
{
    QDBusInterface iface(service, path, "org.freedesktop.DBus.Introspectable", connection);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "Cannot introspect object %s at %s:\n%s (%s)\n",
                qPrintable(path), qPrintable(service),
                qPrintable(err.name()), qPrintable(err.message()));
        exit(1);
    }
    QDBusReply<QString> xml = iface.call("Introspect");

    if (!xml.isValid())
        return;                 // silently

    QDomDocument doc;
    doc.setContent(xml);
    QDomElement node = doc.documentElement();
    QDomElement child = node.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == QLatin1String("interface")) {
            QString ifaceName = child.attribute("name");
            if (QDBusUtil::isValidInterfaceName(ifaceName))
                listInterface(service, path, ifaceName);
            else {
                qWarning("Invalid D-BUS interface name '%s' found while parsing introspection",
                         qPrintable(ifaceName));
            }
        }
        child = child.nextSiblingElement();
    }
}

QStringList readList(int &argc, const char *const *&argv)
{
    --argc;
    ++argv;

    QStringList retval;
    while (argc && QLatin1String(argv[0]) != ")")
        retval += QString::fromLocal8Bit(argv[0]);

    return retval;
}

void placeCall(const QString &service, const QString &path, const QString &interface,
               const QString &member, int argc, const char *const *argv)
{
    QDBusInterface iface(service, path, interface, connection);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        fprintf(stderr, "Interface '%s' not available in object %s at %s:\n%s (%s)\n",
                qPrintable(interface), qPrintable(path), qPrintable(service),
                qPrintable(err.name()), qPrintable(err.message()));
        exit(1);
    }

    const QMetaObject *mo = iface.metaObject();
    QByteArray match = member.toLatin1();
    match += '(';

    int midx = -1;
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);
        QByteArray signature = mm.signature();
        if (signature.startsWith(match)) {
            midx = i;
            break;
        }
    }

    if (midx == -1) {
        fprintf(stderr, "Cannot find '%s.%s' in object %s at %s\n",
                qPrintable(interface), qPrintable(member), qPrintable(path),
                qPrintable(service));
        exit(1);
    }

    QMetaMethod mm = mo->method(midx);
    QList<QByteArray> types = mm.parameterTypes();
    for (int i = 0; i < types.count(); ++i)
        if (types.at(i).endsWith('&')) {
            // reference (and not a reference to const): output argument
            // we're done with the inputs
            while (types.count() > i)
                types.removeLast();
            break;
        }        

    QVariantList params;
    for (int i = 0; argc && i < types.count(); ++i) {
        int id = QVariant::nameToType(types.at(i));
        if ((id == QVariant::UserType || id == QVariant::Map) && types.at(i) != "QDBusVariant") {
            fprintf(stderr, "Sorry, can't pass arg of type %s yet\n",
                    types.at(i).constData());
            exit(1);
        }
        if (id == QVariant::UserType)
            id = QMetaType::type(types.at(i));

        Q_ASSERT(id);

        QVariant p;
        if ((id == QVariant::List || id == QVariant::StringList) && QLatin1String("(") == argv[0])
            p = readList(argc, argv);
        else
            p = QString::fromLocal8Bit(argv[0]);

        if (id < int(QVariant::UserType)) {
            // avoid calling it for QVariant
            p.convert( QVariant::Type(id) );
            if (p.type() == QVariant::Invalid) {
                fprintf(stderr, "Could not convert '%s' to type '%s'.\n",
                        argv[0], types.at(i).constData());
                exit(1);
            }
        } else if (types.at(i) == "QDBusVariant") {
            QDBusVariant tmp(p);
            p = qVariantFromValue(tmp);
        }
        params += p;
        --argc;
        ++argv;
    }
    if (params.count() != types.count() || argc != 0) {
        fprintf(stderr, "Invalid number of parameters\n");
        exit(1);
    }

    QDBusMessage reply = iface.callWithArgs(member, params, QDBus::Block);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        QDBusError err = reply;
        printf("Error: %s\n%s\n", qPrintable(err.name()), qPrintable(err.message()));
        exit(2);
    } else if (reply.type() != QDBusMessage::ReplyMessage) {
        fprintf(stderr, "Invalid reply type %d\n", int(reply.type()));
        exit(1);
    }
    
    foreach (QVariant v, reply) {
        if (v.userType() == QVariant::StringList) {
            foreach (QString s, v.toStringList())
                printf("%s\n", qPrintable(s));
        } else {
            if (v.userType() == qMetaTypeId<QDBusVariant>())
                v = qvariant_cast<QDBusVariant>(v).value;
            printf("%s\n", qPrintable(v.toString()));
        }
    }

    exit(0);
}

bool splitInterfaceAndName(const QString &interfaceAndName, const char *type,
                           QString &interface, QString &member)
{
    interface = interfaceAndName;
    int pos = interface.lastIndexOf(QLatin1Char('.'));
    if (pos != -1) {
        member = interface.mid(pos + 1);
        interface.truncate(pos);
    }

    if (!QDBusUtil::isValidInterfaceName(interface)) {
        fprintf(stderr, "Interface '%s' is not a valid interface name.\n", qPrintable(interface));
        return false;
    } else if (!QDBusUtil::isValidMemberName(member)) {
        fprintf(stderr, "%s name '%s' is not a valid member name.\n", type, qPrintable(member));
        return false;
    }
    return true;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if (argc >= 1 && qstrcmp(argv[1], "--system") == 0) {
        connection = QDBus::systemBus();
        --argc;
        ++argv;
    } else
        connection = QDBus::sessionBus();

    if (!connection.isConnected()) {
        fprintf(stderr, "Could not connect to D-Bus server: %s: %s\n",
                qPrintable(connection.lastError().name()),
                qPrintable(connection.lastError().message()));
        return 1;
    }
    QDBusConnectionInterface *bus = connection.interface();

    if (argc == 1) {
        QStringList names = bus->registeredServiceNames();
        foreach (QString name, names)
            printf("%s\n", qPrintable(name));
        exit(0);
    }
    
    QString service = QLatin1String(argv[1]);
    if (!QDBusUtil::isValidBusName(service)) {
        fprintf(stderr, "Service '%s' is not a valid name.\n", qPrintable(service));
        exit(1);
    }
    if (!bus->isServiceRegistered(service)) {
        fprintf(stderr, "Service '%s' does not exist.\n", qPrintable(service));
        exit(1);
    }

    if (argc == 2) {
        printf("/\n");
        listObjects(service, QString());
        exit(0);
    }

    QString path = QLatin1String(argv[2]);
    if (!QDBusUtil::isValidObjectPath(path)) {
        fprintf(stderr, "Path '%s' is not a valid path name.\n", qPrintable(path));
        exit(1);
    }
    if (argc == 3) {
        listAllInterfaces(service, path);
        exit(0);
    }

    QString interface = QLatin1String(argv[3]);
    QString member;
    int pos = interface.lastIndexOf(QLatin1Char('.'));
    if (pos == -1) {
        member = interface;
        interface.clear();
    } else {
        member = interface.mid(pos + 1);
        interface.truncate(pos);
    }
    if (!interface.isEmpty() && !QDBusUtil::isValidInterfaceName(interface)) {
        fprintf(stderr, "Interface '%s' is not a valid interface name.\n", qPrintable(interface));
        exit(1);
    }
    if (!QDBusUtil::isValidMemberName(member)) {
        fprintf(stderr, "Method name '%s' is not a valid member name.\n", qPrintable(member));
        exit(1);
    }

    placeCall(service, path, interface, member, argc - 4, argv + 4);
}

