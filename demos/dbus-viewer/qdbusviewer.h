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

#ifndef QDBUSVIEWER_H
#define QDBUSVIEWER_H

#include <QtGui/QtGui>
#include <QtDBus/QtDBus>

class QTreeWidget;
class QDomDocument;
class QDomElement;

struct BusSignature
{
    QString mService, mPath, mInterface, mName;
};

class QDBusViewer: public QWidget
{
    Q_OBJECT
public:
    QDBusViewer(const QDBusConnection &connection, QWidget *parent = 0);

public slots:
    void refresh();

private slots:
    void serviceChanged(QTreeWidgetItem *item);
    void prefetchGrandChildren(QTreeWidgetItem *item);
    void showContextMenu(const QPoint &);
    void connectionRequested(const BusSignature &sig);
    void callMethod(const BusSignature &sig);
    void dumpMessage(const QDBusMessage &msg);

    void serviceRegistered(const QString &service);
    void serviceUnregistered(const QString &service);
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

private:
    void prefetchChildren(QTreeWidgetItem *item);
    void addPath(QTreeWidgetItem *parent);
    QDomDocument introspect(const QString &path);
    void addMethods(QTreeWidgetItem *parent, const QDomElement &iface);
    void logMessage(const QString &msg);
    void logError(const QString &msg);

    QDBusConnection c;
    QString currentService;
    QFont interfaceFont;
    QTreeWidget *tree;
    QTreeWidget *services;
    QTextEdit *log;
};

#endif
