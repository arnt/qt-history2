#include "qlistview_.h"

QListView_::QListView_(QWidget *parent, const char *name)
    : QGenericListView(new QListModel, parent, name)
{
    model()->setParent(this);
}

QListView_::QListView_(QListModel *model, QWidget *parent, const char *name)
    : QGenericListView(model, parent, name)
{
}

QListView_::~QListView_()
{
}

void QListView_::setText(int row, const QString &text)
{
    list()->setText(row, text);
}

void QListView_::setIconSet(int row, const QIconSet &iconSet)
{
    list()->setIconSet(row, iconSet);
}

QString QListView_::text(int row) const
{
    return list()->text(row);
}

QIconSet QListView_::iconSet(int row) const
{
    return list()->iconSet(row);
}
