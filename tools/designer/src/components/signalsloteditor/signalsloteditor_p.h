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

#ifndef SIGNALSLOTEDITOR_P_H
#define SIGNALSLOTEDITOR_P_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QAbstractItemModel>

#include <connectionedit.h>

class QDesignerFormWindowInterface;
class DomConnection;

namespace qdesigner { namespace components { namespace signalsloteditor {

class SignalSlotEditor;

struct ClassInfo
{
    ClassInfo(const QString &_class_name = QString(),
                const QStringList &_member_list = QStringList())
        : class_name(_class_name), member_list(_member_list) {}
    QString class_name;
    QStringList member_list;
};
typedef QList<ClassInfo> ClassList;
enum MemberType { SignalMember, SlotMember };

QStringList objectNameList(QDesignerFormWindowInterface *form);
QStringList memberList(QDesignerFormWindowInterface *form, QWidget *widget,
                        MemberType member_type);
bool signalMatchesSlot(const QString &signal, const QString &slot);
ClassList classList(const QString &obj_name, MemberType member_type,
                            const QString &peer, QDesignerFormWindowInterface *form);

class SignalSlotConnection : public Connection
{
public:
    SignalSlotConnection(ConnectionEdit *edit, QWidget *source = 0, QWidget *target = 0);

    void setSignal(const QString &signal);
    void setSlot(const QString &slot);

    QString sender() const;
    QString receiver() const;
    inline QString signal() const { return m_signal; }
    inline QString slot() const { return m_slot; }

    DomConnection *toUi() const;

    virtual void updateVisibility();

private:
    QString m_signal, m_slot;
};

class ConnectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ConnectionModel(SignalSlotEditor *editor, QObject *parent = 0);

    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &data, int role = Qt::DisplayRole);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    QModelIndex connectionToIndex(Connection *con) const;
    Connection *indexToConnection(const QModelIndex &index) const;

private slots:
    void connectionAdded(Connection *con);
    void connectionRemoved(int idx);
    void aboutToRemoveConnection(Connection *con);
    void aboutToAddConnection(Connection *con);
    void connectionChanged(Connection *con);

private:
    SignalSlotEditor *m_editor;
};

} // namespace signalsloteditor
} // namespace components
} // namespace qdesigner

#endif // SIGNALSLOTEDITOR_P_H
