#include "qtreemodel.h"

QTreeModelItem::QTreeModelItem()
    : par(0), mod(0), c(0), edit(true), select(true)
{
}

QTreeModelItem::QTreeModelItem(const QVariantList &elements)
    : par(0), mod(0), c(0), edit(true), select(true)
{
    QVariantList::ConstIterator it = elements.begin();
    for (int e = 0; e < elements.count(); ++it, ++e) {
	if ((*it).type() == QVariant::String)
	    setText(0, (*it).toString());
	else if ((*it).type() == QVariant::IconSet)
	    setIconSet(0, (*it).toIconSet());
    }
}

QTreeModelItem::QTreeModelItem(QTreeModel *model)
    : par(0), mod(model), c(0), edit(true), select(true)
{
    if (model)
	model->append(this);
}

QTreeModelItem::QTreeModelItem(QTreeModelItem *parent)
    : par(parent), mod(parent->mod), c(0), edit(true), select(true)
{
    if (parent)
	parent->children.push_back(this);
    model()->emitContentsInserted(this);
}

QTreeModelItem::~QTreeModelItem()
{
    for (int i = 0; i < children.count(); ++i)
	delete children.at(i);
}

void QTreeModelItem::setColumnCount(int columns)
{
    c = columns;
    txt.resize(c);
    icn.resize(c);
}

void QTreeModelItem::setText(int column, const QString &text)
{
    if (column >= columnCount())
	setColumnCount(column + 1);
    txt[column] = text;
}

void QTreeModelItem::setIconSet(int column, const QIconSet &iconSet)
{
    if (column >= columnCount())
	setColumnCount(column + 1);
    icn[column] = iconSet;
}

QTreeModel::QTreeModel(int columns, QObject *parent, const char *name)
    : QGenericItemModel(parent, name), c(0)
{
    setColumnCount(columns);
}

QTreeModel::~QTreeModel()
{
    for (int i = 0; i < tree.count(); ++i)
	delete tree.at(i);
}

void QTreeModel::setColumnCount(int columns)
{
    if (c == columns)
	return;
    int _c = c;
    c = columns;
    topHeader.setColumnCount(c);
    for (int i = _c; i < c; ++i)
	topHeader.setText(i, QString::number(i));
    int r = rowCount(0);
    if (c > _c)
	emit contentsInserted(index(0, _c - 1, 0), index(r - 1, c - 1, 0));
    else
	emit contentsRemoved(0, index(0, c - 1, 0), index(r - 1, _c - 1, 0));
}

int QTreeModel::columnCount() const
{
    return c;
}
/*
void QTreeModel::setText(const QModelIndex &index, int column, const QString &text)
{
    if (column >= columnCount(QModelIndex()))
	return;
    QTreeModelItem *itm = item(index);
    if (itm)
	itm->setText(column, text);
}

void QTreeModel::setIconSet(const QModelIndex &index, int column, const QIconSet &iconSet)
{
    if (column >= columnCount(QModelIndex()))
	return;
    QTreeModelItem *itm = item(index);
    if (itm)
	itm->setIconSet(column, iconSet);
}

QString QTreeModel::text(const QModelIndex &index, int column) const
{
    if (column >= columnCount(QModelIndex()))
	return QString();
    QTreeModelItem *itm = item(index);
    if (itm)
	return itm->text(column);
    return QString();
}

QIconSet QTreeModel::iconSet(const QModelIndex &index, int column) const
{
    if (column >= columnCount(QModelIndex()))
	return QIconSet();
    QTreeModelItem *itm = item(index);
    if (itm)
	return itm->iconSet(column);
    return QIconSet();
}
*/
void QTreeModel::setColumnText(int column, const QString &text)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    int e = element(index, QVariant::String);
    setData(index, e, QVariant(text));
}

void QTreeModel::setColumnIconSet(int column, const QIconSet &iconSet)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    int e = element(index, QVariant::IconSet);
    setData(index, e, QVariant(iconSet));
}

QString QTreeModel::columnText(int column) const
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    int e = element(index, QVariant::String);
    return data(index, e).toString();
}

QIconSet QTreeModel::columnIconSet(int column) const
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    int e = element(index, QVariant::IconSet);
    return data(index, e).toIconSet();
}

QTreeModelItem *QTreeModel::item(const QModelIndex &index) const
{
    if (!index.isValid())
	return 0;
    if (index.type() != QModelIndex::View)
	return &topHeader;
    return (QTreeModelItem *)index.data();
}

QModelIndex QTreeModel::index(QTreeModelItem *item) const
{
    if (!item)
	return QModelIndex();
    const QTreeModelItem *par = item->parent();
    int row = par ? par->children.indexOf(item) : tree.indexOf(item);
    return QModelIndex(row, 0, item);
}

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent,
                              QModelIndex::Type type) const
{
    int r = tree.count();
    if (row < 0 || row >= r || column < 0 || column >= c)
	return QModelIndex();
    if (!parent.isValid() && row < r) {// toplevel
	QTreeModelItem *itm = ((QTreeModel*)this)->tree[row]; // FIXME
	return QModelIndex(row, column, itm, type);
    }
    QTreeModelItem *parentItem = item(parent);
    if (parentItem && row < parentItem->childCount()) {
	QTreeModelItem *itm = (QTreeModelItem*)parentItem->child(row); // FIXME
	return QModelIndex(row, column, itm, type);
    }
    return QModelIndex();
}

QModelIndex QTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
	return QModelIndex();
    const QTreeModelItem *itm = (const QTreeModelItem*)child.data();
    if (!itm)
	return QModelIndex();
    QTreeModelItem *parent = (QTreeModelItem*)itm->parent(); // FIXME
    return index(parent);
}

int QTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
	QTreeModelItem *parentItem = item(parent);
	if (parentItem)
	    return parentItem->childCount();
    }
    return tree.count();
}

int QTreeModel::columnCount(const QModelIndex &) const
{
    return c;
}

QVariant QTreeModel::data(const QModelIndex &index, int element) const
{
    if (!index.isValid())
	return QVariant();

    QTreeModelItem *itm = item(index);
    if (!itm)
	return QVariant();
    if (element == 0)
	return itm->text(index.column());
    if (element == 1)
	return itm->iconSet(index.column());
    return QVariant();
}

void QTreeModel::setData(const QModelIndex &index, int element, const QVariant &value)
{
    if (!index.isValid())
	return;
    QTreeModelItem *itm = item(index);
    if (!itm)
	return;
    if (element == 0)
	itm->setText(index.column(), value.toString());
    else if (element == 1)
	itm->setIconSet(index.column(), value.toIconSet());
    emit contentsChanged(index, index);
}

void QTreeModel::insertData(const QModelIndex &index, const QVariantList &elements)
{
    QTreeModelItem *sibling = item(index);
    QTreeModelItem *parent = (QTreeModelItem *)sibling->parent();
    QTreeModelItem *itm = new QTreeModelItem(elements);
    if (parent) {
	itm->par = parent;
	itm->mod = this;
	int i = parent->children.indexOf(sibling);
	parent->children.insert(i, itm);
    } else {
	itm->mod = this;
	int i = tree.indexOf(sibling);
	tree.insert(i, itm);
    }
    emitContentsInserted(itm);
}

void QTreeModel::appendData(const QVariantList &elements)
{
    append(new QTreeModelItem(elements));
}

QVariant::Type QTreeModel::type(const QModelIndex &index, int element) const
{
    if (!index.isValid())
	return QVariant::Invalid;
    if (element == 0)
	return QVariant::String;
    else if (element == 1 && index.type() == QModelIndex::View)
	return QVariant::IconSet;
    return QVariant::Invalid;
}

int QTreeModel::element(const QModelIndex &index, QVariant::Type type) const
{
    if (!index.isValid())
	return -1;
    if (type == QVariant::String)
	return 0;
    else if (type == QVariant::IconSet && index.type() == QModelIndex::View)
	return 1;
    return -1;
}

bool QTreeModel::isSelectable(const QModelIndex &) const
{
    return true;
}

bool QTreeModel::isEditable(const QModelIndex &) const
{
    return true;
}

void QTreeModel::append(QTreeModelItem *item)
{
    tree.push_back(item);
    int r = tree.count();
    QModelIndex topLeft = index(r - 1, 0, 0);
    QModelIndex bottomRight = index(r - 1, c - 1, 0);
    emit contentsInserted(topLeft, bottomRight);
}

void QTreeModel::emitContentsInserted(QTreeModelItem *item)
{
    QModelIndex topLeft = index(item);
    QModelIndex parentIndex = parent(topLeft);
    QModelIndex bottomRight = index(topLeft.row(), columnCount(parentIndex) - 1, parentIndex);
    emit contentsInserted(topLeft, bottomRight);
}
