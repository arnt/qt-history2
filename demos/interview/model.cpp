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

#include "model.h"
#include <qiconset.h>

static QString i2s(QChar *buf, int size, int num)
{
    static ushort unicode_zero = QChar('0').unicode();
    static ushort unicode_dash = QChar('-').unicode();
    bool neg = num < 0;
    int len = 0;
    while (num != 0 && size > 0) {
        buf[--size] = unicode_zero + (num % 10);
        num /= 10;
        ++len;
    }
    if (len == 0) {
        buf[--size] = unicode_zero;
        ++len;
    }
    if (neg) {
        buf[--size] = unicode_dash;
        ++len;
    }
    return QString(&buf[size], len);
}


Model::Model(int rows, int columns, QObject *parent)
    : QAbstractItemModel(parent),
      rc(rows), cc(columns),
      tree(new QVector<Node>(rows, Node(0)))
{

}

Model::~Model()
{
    delete tree;
}

QModelIndex Model::index(int row, int column, const QModelIndex &parent, QModelIndex::Type type) const
{
    if (row < rc && row >= 0 && column < cc && column >= 0) {
	Node *n = node(row, static_cast<Node*>(parent.data()));
	if (n)
	    return createIndex(row, column, n, type);
    }
    return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        Node *n = static_cast<Node*>(child.data());
        Node *p = parent(n);
        if (p)
            return createIndex(row(p), 0, p);
    }
    return QModelIndex();
}

int Model::rowCount(const QModelIndex &) const
{
    return rc;
}

int Model::columnCount(const QModelIndex &) const
{
    return cc;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    static QIconSet folder(QPixmap("folder.png"));
    static QIconSet service(QPixmap("services.png"));
    
    if (index.type() == QModelIndex::VerticalHeader) {
        if (role == DisplayRole)
            return i2s(strbuf, 65, index.row());
        if (role == DecorationRole)
            return service;
        return QVariant();
    }
    
    if (index.type() == QModelIndex::HorizontalHeader) {
        if (role == DisplayRole)
            return i2s(strbuf, 65, index.column());
        if (role == DecorationRole)
            return service;
        return QVariant();
    }

    if (role == DisplayRole)
	return "Item " + i2s(strbuf, 65, index.row()) + ":" + i2s(strbuf, 65, index.column());
    if (role == DecorationRole)
	return folder;
    return QVariant();
}

bool Model::hasChildren(const QModelIndex &) const
{
    return rc > 0 && cc > 0;
}

bool Model::isDragEnabled(const QModelIndex &index) const
{
    return index.isValid();
}

Model::Node *Model::node(int row, Node *parent) const
{
    if (parent && !parent->children)
	parent->children = new QVector<Node>(rc, Node(parent));
    QVector<Node> *v = parent ? parent->children : tree;
    return const_cast<Node*>(&(v->at(row)));
}

Model::Node *Model::parent(Node *child) const
{
    return child ? child->parent : 0;
}

int Model::row(Node *node) const
{
     const Node *first = node->parent ? &(node->parent->children->at(0)) : &(tree->at(0));
     return (node - first);
}
