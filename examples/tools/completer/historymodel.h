/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QCompleter>
#include <QDirModel>
#include <QList>
#include <QPair>

class HistoryCompleter : public QCompleter
{
    Q_OBJECT

public:
    HistoryCompleter(QObject *parent = 0);

    QStringList splitPath(const QString& filter) const;    
};

class HistoryModel : public QDirModel
{
    Q_OBJECT
public:
    HistoryModel(QObject *parent = 0);

    
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

public slots:
    void addToHistory(const QString& display, QString edit = QString());
    void addToHistory();

private:
    QList<QPair<QString, QString> > history;
};

#endif // HISTORYMODEL_H

