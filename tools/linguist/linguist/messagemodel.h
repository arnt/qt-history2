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

#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <metatranslator.h>
#include <qabstractitemmodel.h>
#include <qlist.h>

class ContextItem;

class MessageItem
{
public:
    MessageItem(const MetaTranslatorMessage &message,
        const QString &text, const QString &comment, ContextItem *ctxtI);
    inline virtual ~MessageItem() {}

    inline virtual bool danger() const {return d;}

    inline void setTranslation(const QString &translation) {m.setTranslation(translation);}
    void setFinished(bool finished);
    void setDanger(bool danger);

    inline void setContextItem(ContextItem *ctxtI) {cntxtItem = ctxtI;}
    inline ContextItem *contextItem() const {return cntxtItem;}

    inline QString context() const {return m.context();}
    inline QString sourceText() const {return tx;}
    inline QString comment() const {return com;}
    inline QString translation() const {return m.translation();}
    inline bool finished() const {return fini;}
    inline MetaTranslatorMessage message() const {return m;}

private:
    MetaTranslatorMessage m;
    QString tx;
    QString com;
    bool fini;
    bool d;
    ContextItem *cntxtItem;
};

class MessageModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MessageModel(QObject *parent = 0);

    ContextItem *contextItem() {return cntxtItem;}
    void setContextItem(ContextItem *ctxtI);
    void updateItem(QModelIndex indx);

    // from qabstracttablemodel
    int rowCount(const QModelIndex &) const;
    int columnCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:
    ContextItem *cntxtItem;
};

#endif //MESSAGEMODEL_H
