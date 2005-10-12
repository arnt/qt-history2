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

#ifndef QSTRINGFILTERMODEL_H
#define QSTRINGFILTERMODEL_H

#include <QtCore/qregexp.h>
#include <QtGui/qfilteringproxymodel.h>

QT_MODULE(Gui)

class QStringFilterModelPrivate;

class Q_GUI_EXPORT QStringFilterModel : public QFilteringProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QRegExp regExp READ regExp WRITE setRegExp)
    Q_PROPERTY(int keyColumn READ keyColumn WRITE setKeyColumn)
public:
    QStringFilterModel(QObject *parent = 0);
    ~QStringFilterModel();

    QRegExp regExp() const;
    void setRegExp(const QRegExp &regExp);

    int keyColumn() const;
    void setKeyColumn(int column);

public Q_SLOTS:
    void setPattern(const QString &pattern); 

protected:
    QStringFilterModel(QStringFilterModelPrivate&, QObject *parent);
    bool filterRow(int source_row, const QModelIndex &source_parent) const;
    void sourceDataChanged(const QModelIndex &source_top_left,
                           const QModelIndex &spirce_bottom_right);

private:
    Q_DECLARE_PRIVATE(QStringFilterModel)
    Q_DISABLE_COPY(QStringFilterModel)
};

#endif // QSTRINGFILTERMODEL_H
