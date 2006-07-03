/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdbusviewer.h"

#include <QtXml/QtXml>

enum { PrefetchRole = 4242, PathRole, MethodNameRole, InterfaceRole };
enum { InterfaceItem = QTreeWidgetItem::UserType + 1, PathItem, MethodItem,
       SignalItem, PropertyItem };

QDBusViewer::QDBusViewer(const QDBusConnection &connection, QWidget *parent)
    : QWidget(parent), c(connection)
{
    services = new QTreeWidget;
    services->setRootIsDecorated(false);
    services->setHeaderLabels(QStringList("Services"));

    tree = new QTreeWidget;
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    tree->setHeaderLabels(QStringList("Methods"));
    interfaceFont = tree->font();
    interfaceFont.setItalic(true);

    QVBoxLayout *topLayout = new QVBoxLayout(this);
    log = new QTextEdit;
    log->setReadOnly(true);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(services, 1);
    layout->addWidget(tree, 2);

    topLayout->addLayout(layout);
    topLayout->addWidget(log);

    connect(services, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(serviceChanged(QTreeWidgetItem*)));
    connect(tree, SIGNAL(itemExpanded(QTreeWidgetItem*)),
            this, SLOT(prefetchGrandChildren(QTreeWidgetItem*)));
    connect(tree, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    QMetaObject::invokeMethod(this, "refresh", Qt::QueuedConnection);

    if (c.isConnected()) {
        logMessage("Connected to D-Bus.");
        QDBusConnectionInterface *iface = c.interface();
        connect(iface, SIGNAL(serviceRegistered(QString)),
                this, SLOT(serviceRegistered(QString)));
        connect(iface, SIGNAL(serviceUnregistered(QString)),
                this, SLOT(serviceUnregistered(QString)));
        connect(iface, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                this, SLOT(serviceOwnerChanged(QString,QString,QString)));
    } else {
        logError("Cannot connect to D-Bus: " + c.lastError().message());
    }
}

void QDBusViewer::logMessage(const QString &msg)
{
    log->append(msg + "\n");
}

void QDBusViewer::logError(const QString &msg)
{
    log->append("<font color=\"red\">Error: </font>" + Qt::escape(msg) + "<br>");
}

void QDBusViewer::refresh()
{
    services->clear();

    QStringList names = c.interface()->registeredServiceNames();
    names.removeAll(c.baseService()); // don't show the viewer itself
    foreach (QString service, names)
        new QTreeWidgetItem(services, QStringList(service));
}

QDomDocument QDBusViewer::introspect(const QString &path)
{
    QDomDocument doc;

    QDBusInterface iface(currentService, path, "org.freedesktop.DBus.Introspectable", c);
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        logError(QString("Cannot introspect object %1 at %2:\n  %3 (%4)\n").arg(path).arg(currentService).arg(err.name()).arg(err.message()));
        return doc;
    }

    QDBusReply<QString> xml = iface.call("Introspect");

    if (!xml.isValid()) {
        logError(QString("Invalid XML received from object %1 at %2\n").arg(path).arg(currentService));
        return doc;
    }

    doc.setContent(xml);
    return doc;
}

void QDBusViewer::addMethods(QTreeWidgetItem *parent, const QDomElement &iface)
{
    QDomElement child = iface.firstChildElement();
    while (!child.isNull()) {
        QTreeWidgetItem *item = 0;
        if (child.tagName() == QLatin1String("method")) {
            item = new QTreeWidgetItem(parent,
                    QStringList("Method: " + child.attribute("name")), MethodItem);
        } else if (child.tagName() == QLatin1String("signal")) {
            item = new QTreeWidgetItem(parent,
                    QStringList("Signal: " + child.attribute("name")), SignalItem);
        } else if (child.tagName() == QLatin1String("property")) {
            item = new QTreeWidgetItem(parent,
                    QStringList("Property: " + child.attribute("name")), PropertyItem);
        } else {
            qDebug() << "addMethods: unknown tag:" << child.tagName();
        }
        if (item) {
            item->setData(0, MethodNameRole, child.attribute("name"));
            item->setData(0, PathRole, parent->data(0, PathRole));
            item->setData(0, InterfaceRole, parent->data(0, InterfaceRole));
        }

        child = child.nextSiblingElement();
    }
}

void QDBusViewer::addPath(QTreeWidgetItem *parent)
{
    QString path;
    if (parent)
        path = parent->data(0, PathRole).toString();

    QDomDocument doc = introspect(path.isEmpty() ? QString::fromLatin1("/") : path);
    QDomElement node = doc.documentElement();
    QDomElement child = node.firstChildElement();
    while (!child.isNull()) {
        if (child.tagName() == QLatin1String("node")) {
            QString sub = child.attribute("name") + QLatin1Char('/');
            QTreeWidgetItem *item;
            if (parent)
                item = new QTreeWidgetItem(parent, QStringList(sub), PathItem);
            else
                item = new QTreeWidgetItem(tree, QStringList(sub), PathItem);
            item->setData(0, PathRole, path + QLatin1Char('/') + child.attribute("name"));
            addMethods(item, child);
        } else if (child.tagName() == QLatin1String("interface")) {
            QString ifaceName = child.attribute("name");
            QTreeWidgetItem *item;
            if (parent)
                item = new QTreeWidgetItem(parent, QStringList(ifaceName), InterfaceItem);
            else
                item = new QTreeWidgetItem(tree, QStringList(ifaceName), InterfaceItem);
            item->setFont(0, interfaceFont);
            item->setData(0, PathRole, path);
            item->setData(0, InterfaceRole, ifaceName);
            addMethods(item, child);
        } else {
            qDebug() << "addPath: Unknown tag name:" << child.tagName();
        }
        child = child.nextSiblingElement();
    }
}

void QDBusViewer::serviceChanged(QTreeWidgetItem *item)
{
    tree->clear();

    currentService.clear();
    if (!item)
        return;
    currentService = item->text(0);

    addPath(0);
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
        prefetchChildren(tree->topLevelItem(i));
}

void QDBusViewer::prefetchGrandChildren(QTreeWidgetItem *item)
{
    if (!item)
        return;

    for (int i = 0; i < item->childCount(); ++i)
        prefetchChildren(item->child(i));
}

void QDBusViewer::prefetchChildren(QTreeWidgetItem *item)
{
    if (!item || item->data(0, PrefetchRole).toBool())
        return;

    item->setData(0, PrefetchRole, true);

    if (item->type() != PathItem)
        return;

    addPath(item);
}

void QDBusViewer::callMethod(const BusSignature &sig)
{
    QDBusInterface iface(sig.mService, sig.mPath, sig.mInterface, c);
    const QMetaObject *mo = iface.metaObject();

    // find the method
    QMetaMethod method;
    for (int i = 0; i < mo->methodCount(); ++i) {
        const QString signature = QString::fromLatin1(mo->method(i).signature());
        if (signature.startsWith(sig.mName) && signature.at(sig.mName.length()) == '(')
            method = mo->method(i);
    }
    if (!method.signature()) {
        QMessageBoxEx::warning(this, "Unable to find method",
                QString("Unable to find method %1 on path %2 in interface %3").arg(
                    sig.mName).arg(sig.mPath).arg(sig.mInterface));
        return;
    }

    QList<QByteArray> paramTypes = method.parameterTypes();
    foreach (QByteArray paramType, paramTypes) {
        if (!QVariant(QVariant::nameToType(paramType)).canConvert(QVariant::String)) {
            QMessageBoxEx::warning(this, "Unable to call method",
                    QString("Cannot marshall parameter of type %1").arg(
                        QVariant::nameToType(paramType)));
        }
    }

    QList<QVariant> arguments;
    if (!paramTypes.isEmpty()) {
        QString input;
        bool ok = true;
        while (arguments.isEmpty()) {
            input = QInputDialog::getText(this, "Arguments",
                    "Please enter the arguments for the call, separated by comma."
                    "<br>Example: <i>hello,world,2.3,-34</i><br><br><b>Signature:</b> "
                    + Qt::escape(QString::fromUtf8(method.signature())),
                    QLineEdit::Normal, input, &ok);
            if (!ok)
                return;

            QStringList argStrings = input.split(',');
            if (argStrings.count() != paramTypes.count())
                continue;

            for (int i = 0; i < argStrings.count(); ++i) {
                QVariant v = argStrings.at(i);
                if (!v.convert(QVariant::nameToType(paramTypes.at(i)))) {
                    arguments.clear();
                    break;
                }
                arguments += v;
            }
        }
    }

    QDBusMessage message = QDBusMessage::methodCall(sig.mService, sig.mPath, sig.mInterface,
            sig.mName, c);
    message.setArguments(arguments);
    c.call(message, this, SLOT(dumpMessage(QDBusMessage)));
}

void QDBusViewer::showContextMenu(const QPoint &point)
{
    QTreeWidgetItem *item = tree->itemAt(point);
    if (!item)
        return;

    BusSignature sig;
    sig.mService = currentService;
    sig.mPath = item->data(0, PathRole).toString();
    if (sig.mPath.isEmpty())
        sig.mPath = "/";
    sig.mInterface = item->data(0, InterfaceRole).toString();
    sig.mName = item->data(0, MethodNameRole).toString();

    QMenu menu;
    switch (item->type()) {
    case SignalItem: {
        QAction *action = new QAction("&Connect", &menu);
        action->setData(0);
        menu.addAction(action);
        break; }
    case MethodItem: {
        QAction *action = new QAction("&Call", &menu);
        action->setData(1);
        menu.addAction(action);
        break; }
#if 0
    case PropertyItem: {
        QAction *actionSet = new QAction("&Set value", &menu);
        actionSet->setData(2);
        QAction *actionGet = new QAction("&Get value", &menu);
        actionGet->setData(3);
        menu.addAction(actionSet);
        menu.addAction(actionGet);
        break; }
#endif
    default:
        return;
    }

    QAction *selectedAction = menu.exec(tree->viewport()->mapToGlobal(point));
    if (!selectedAction)
        return;

    switch (selectedAction->data().toInt()) {
    case 0:
        connectionRequested(sig);
        break;
    case 1:
        callMethod(sig);
        break;
    }
}

void QDBusViewer::connectionRequested(const BusSignature &sig)
{
    c.connect(sig.mService, sig.mPath, sig.mInterface, sig.mName, this,
              SLOT(dumpMessage(QDBusMessage)));
}

void QDBusViewer::dumpMessage(const QDBusMessage &message)
{
    QList<QVariant> args = message.arguments();
    QString out = "Received ";

    switch (message.type()) {
    case QDBusMessage::SignalMessage:
        out += "signal ";
        break;
    case QDBusMessage::ErrorMessage:
        out += "error message ";
        break;
    case QDBusMessage::ReplyMessage:
        out += "reply ";
        break;
    default:
        out += "message ";
        break;
    }

    out += "from ";
    out += message.service();
    if (!message.path().isEmpty())
        out += ", path " + message.path();
    if (!message.interface().isEmpty())
        out += ", interface <i>" + message.interface() + "</i>";
    if (!message.member().isEmpty())
        out += ", member " + message.member();
    out += "<br>";
    if (!args.isEmpty()) {
        out += "&nbsp;&nbsp;Parameters: ";
        foreach (QVariant arg, args) {
            if (!arg.canConvert(QVariant::String)) {
                out += "[";
                out += arg.typeName();
                out += "]";
            } else {
                out += "<b>\"</b>" + Qt::escape(arg.toString()) + "<b>\"</b>";
            }
            out += ", ";
        }
        out.chop(2);
    }

    log->append(out);
}

void QDBusViewer::serviceRegistered(const QString &service)
{
    if (service == c.baseService())
        return;

    new QTreeWidgetItem(services, QStringList(service));
}

static QTreeWidgetItem *findItem(const QTreeWidget *services, const QString &name)
{
    for (int i = 0; i < services->topLevelItemCount(); ++i) {
        if (services->topLevelItem(i)->text(0) == name)
            return services->topLevelItem(i);
    }
    return 0;
}

void QDBusViewer::serviceUnregistered(const QString &name)
{
    delete findItem(services, name);
}

void QDBusViewer::serviceOwnerChanged(const QString &name, const QString &oldOwner,
                                      const QString &newOwner)
{
    QTreeWidgetItem *item = findItem(services, name);

    if (!item && oldOwner.isEmpty() && !newOwner.isEmpty())
        serviceRegistered(name);
    else if (item && !oldOwner.isEmpty() && newOwner.isEmpty())
        delete item;
    else if (item && !oldOwner.isEmpty() && !newOwner.isEmpty()) {
        delete item;
        serviceRegistered(name);
    }
}

