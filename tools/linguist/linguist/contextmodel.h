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

#ifndef CONTEXTMODEL_H
#define CONTEXTMODEL_H

#include "messagemodel.h"

#include <qabstractitemmodel.h>
#include <qlist.h>

class MessageModel;
class ContextModel;

class ContextItem
{
public:
    ContextItem(QString c);
    ~ContextItem();

    inline bool danger() const {return dangerCount > 0;}

    void appendToComment(const QString& x);
    inline void incrementUnfinishedCount() {++unfinishedCount;}
    inline void decrementUnfinishedCount() {--unfinishedCount;}
    inline void incrementDangerCount() {++dangerCount;}
    inline void decrementDangerCount() {--dangerCount;}
    inline void incrementObsoleteCount() {++obsoleteCount;}
    inline bool isContextObsolete() const {return (obsoleteCount == msgItemList.count());}

    inline int unfinished() const {return unfinishedCount;}
    inline int obsolete() const {return obsoleteCount;}

    inline QString context() const {return ctxt;}
    inline QString comment() const {return com;}
    inline QString fullContext() const {return com.trimmed();}
    inline bool finished() const {return unfinishedCount == 0;}

    MessageItem *messageItem(int i);
    inline const QList<MessageItem *> messageItemList() const {return msgItemList;}
    inline void appendMessageItem(MessageItem *msgItem) {msgItemList.append(msgItem);}
    inline int messageItemsInList() const {return msgItemList.count();}

    static bool compare(const MessageItem *left, const MessageItem *right);
    void sortMessages(int column, Qt::SortOrder order);
    bool sortParameters(Qt::SortOrder &so, int &sc) const;

private:
    Qt::SortOrder sortOrder;
    int sortColumn;
    QString com;
    QString ctxt;
    int unfinishedCount;
    int dangerCount;
    int obsoleteCount;
    QList<MessageItem *> msgItemList;
};

class ContextModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ContextModel(QObject *parent = 0);

    ContextItem *contextItem(const QModelIndex &indx) const;
    inline const QList<ContextItem *> contextList() const {return cntxtList;}
    inline void appendContextItem(ContextItem *cntxtItem) {cntxtList.append(cntxtItem);}
    inline int contextsInList() const {return cntxtList.count();}

    void updateItem(QModelIndex indx);
    void updateAll();
    void clearContextList();

    static bool compare(const ContextItem *left, const ContextItem *right);
    bool sortParameters(Qt::SortOrder &so, int &sc) const;

    // from abstracttablemodel
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    int rowCount(const QModelIndex &) const;
    int columnCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
private:
    Qt::SortOrder sortOrder;
    int sortColumn;

    QList<ContextItem *> cntxtList;
};

#endif //CONTEXTMODEL_H
