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

#ifndef PHRASEMODEL_H
#define PHRASEMODEL_H

#include "phrase.h"

#include <qlist.h>
#include <qabstractitemmodel.h>

class PhraseModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PhraseModel(QObject *parent = 0)
        : QAbstractTableModel(parent) {}

    void removePhrases();
    QList<Phrase> phraseList() const {return plist;}

    QModelIndex addPhrase(Phrase p);
    void removePhrase(const QModelIndex &index);

    Phrase phrase(const QModelIndex &index) const;
    void setPhrase(const QModelIndex &indx, Phrase ph);
    QModelIndex index(const Phrase phr) const;

    static bool compare(const Phrase left, const Phrase right);
    bool sortParameters(Qt::SortOrder &so, int &sc) const;
    void resort();

    // from qabstracttablemodel
    int rowCount() const;
    int columnCount() const;
    QVariant data(const QModelIndex &index, int role = DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = DisplayRole) const;

    virtual bool isSortable() const {return true;}
    void sort(int column, const QModelIndex &parent = QModelIndex::Null,
        Qt::SortOrder order = Qt::AscendingOrder);

private:
    Qt::SortOrder sortOrder;
    int sortColumn;

    QList<Phrase> plist;
};

#endif //PHRASEMODEL_H
