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

#ifndef QDBUSMODEL_H
#define QDBUSMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtDBus/QtDBus>

struct QDBusItem;
class QDomDocument;
class QDomElement;

class QDBusModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Type { InterfaceItem, PathItem, MethodItem, SignalItem, PropertyItem };

    QDBusModel(const QString &service, const QDBusConnection &connection);
    ~QDBusModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Type itemType(const QModelIndex &index) const;
    QString dBusPath(const QModelIndex &index) const;
    QString dBusInterface(const QModelIndex &index) const;
    QString dBusMethodName(const QModelIndex &index) const;

    void refresh(const QModelIndex &index = QModelIndex());

    QModelIndex findObject(const QDBusObjectPath &objectPath);

Q_SIGNALS:
    void busError(const QString &text);

private:
    QDomDocument introspect(const QString &path);
    void addMethods(QDBusItem *parent, const QDomElement &iface);
    void addPath(QDBusItem *parent);

    QString service;
    QDBusConnection c;
    QDBusItem *root;
};

#endif

