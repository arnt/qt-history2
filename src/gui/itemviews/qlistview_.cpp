#include "qlistview_.h"

QListView_::QListView_(QWidget *parent)
    : QGenericListView(new QListModel, parent)
{
    model()->setParent(this);
}

QListView_::QListView_(QListModel *model, QWidget *parent)
    : QGenericListView(model, parent)
{
}

QListView_::~QListView_()
{
}

void QListView_::setText(int row, const QString &text)
{
    model()->setText(row, text);
}

void QListView_::setIconSet(int row, const QIconSet &iconSet)
{
    model()->setIconSet(row, iconSet);
}

QString QListView_::text(int row) const
{
    return model()->text(row);
}

QIconSet QListView_::iconSet(int row) const
{
    return model()->iconSet(row);
}
