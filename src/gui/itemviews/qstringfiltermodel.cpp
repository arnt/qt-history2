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
  \ingroup model-view

  Although the model/view framework is designed to handle data in arbitrary formats,
  many models provide textual representations of their data to other components, as
  described by \l{Qt::DisplayRole}. As a result, it is often possible to categorize
  and filter data using this role, even if the primary data supplied by the model is
  supplied in another format.

  If you need to apply more specialized filters to data, subclass QFilteringProxyModel
  and reimplement its virtual functions to provide the sorting behavior you require.

  QStringFilterModel enables the developer to filter out rows at any level in a
  hierarchical model by applying a \l{QRegExp}{regular expression}, specified by the 
  \l regExp property, to the data supplied by the source model. Only one item from each
  row in the model is tested against the regular expression, and this is obtained from
  the column specified by the \l keyColumn property; by default the first column is
  used (the key column is 0). Rows where the regular expression fails to match against
  the data obtained from the key column are filtered out, and are not supplied to views
  and delegates.

  Additionally, the pattern used for the regular expression can be set using the
  setPattern() slot. For example, a QLineEdit can be used to allow the user to specify
  which regular expression to apply to the model's data:

  \quotefromfile snippets/qstringfiltermodel/main.cpp
  \skipto QStringFilterModel
  \printuntil setSourceModel
  \skipto QLineEdit
  \printuntil QLineEdit
  \skipto connect
  \printuntil setPattern

  The model applies the regular expression whenever the source model or the regular
  expression changes, ensuring that the data displayed in other components is accurate
  and up-to-date.

  \sa QFilteringProxyModel, QAbstractProxyModel, QAbstractItemModel, {Model/View Programming}
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
  Sets the string pattern used to filter the contents
  of the source model, to \a pattern.
*/
void QStringFilterModel::setPattern(const QString &pattern)
{
    d_func()->regExp.setPattern(pattern);
    clear();
}

/*!
  \reimp
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
