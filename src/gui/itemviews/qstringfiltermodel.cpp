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

#include "qstringfiltermodel.h"
#include <qdebug.h>
#include <private/qfilteringproxymodel_p.h>

/*!
  \class QStringFilterModel
  \brief The QStringFilterModel class provides support for filtering data
  that is passed between another model and a view, using a QRegExp.

*/

class QStringFilterModelPrivate : public QFilteringProxyModelPrivate
{
    //Q_DECLARE_PUBLIC(QStringFilterModel)
public:
    QStringFilterModelPrivate() : QFilteringProxyModelPrivate(), column(0) {}
    QRegExp regExp;
    int column;
};

/*!
    Constructs a string filterting model with the given \a parent.
*/
QStringFilterModel::QStringFilterModel(QObject *parent)
    : QFilteringProxyModel(*new QStringFilterModelPrivate(), parent)
{

}

/*!
    \internal
*/
QStringFilterModel::QStringFilterModel(QStringFilterModelPrivate &dd, QObject *parent)
    : QFilteringProxyModel(dd, parent)
{

}

/*!
    Destroys the string filtering model.
*/
QStringFilterModel::~QStringFilterModel()
{

}

/*!
  \property QStringFilterModel::regExp
  \brief the regular expression used to filter the contents
  of the source model.

*/
QRegExp QStringFilterModel::regExp() const
{
    return d_func()->regExp;
}

void QStringFilterModel::setRegExp(const QRegExp &regExp)
{
    d_func()->regExp = regExp;
    clear();
}

/*!
  \property QStringFilterModel::keyColumn
  \brief the column where the key used to filter the contents
  of the source model is read from.

*/
int QStringFilterModel::keyColumn() const
{
    return d_func()->column;
}

void QStringFilterModel::setKeyColumn(int column)
{
    d_func()->column = column;
    clear();
}

/*!
  Sets the string pattern used to filterthe contents
  of the source model.
*/
void QStringFilterModel::setPattern(const QString &pattern)
{
    d_func()->regExp.setPattern(pattern);
    clear();
}

/*!
  Returns true if the value in the item in the row indicated by
  the given \a source_row and \a source_parent should be removed from the model.
*/
bool QStringFilterModel::filterRow(int source_row, const QModelIndex &source_parent) const
{
    if (d_func()->regExp.isEmpty())
        return false;
    QModelIndex source_index = sourceModel()->index(source_row, d_func()->column, source_parent);
    Q_ASSERT(source_index.isValid());
    QString key = sourceModel()->data(source_index, Qt::DisplayRole).toString();
    return !key.contains(d_func()->regExp);
}

/*!
  \reimp
*/
void QStringFilterModel::sourceDataChanged(const QModelIndex &source_top_left,
                                           const QModelIndex &spirce_bottom_right)
{
    clear(); // the changed data may change the filtering
    QFilteringProxyModel::sourceDataChanged(source_top_left, spirce_bottom_right);
}

#include "moc_qstringfiltermodel.cpp"
