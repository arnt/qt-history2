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
#include <qpixmap.h>
#include <private/qabstractitemmodel_p.h>

Model::Model(int rows, int columns, int depth, QObject *parent)
    : QAbstractItemModel(parent),
      rc(rows), cc(columns), d(depth),
      tree(new QVector<Node>(rows, Node(0)))
{

}

Model::~Model()
{
    delete tree;
}

QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
{
    if (row < rc && row >= 0 && column < cc && column >= 0) {
        Node *pn = static_cast<Node*>(parent.data());
        Node *n = (depth(pn) >= d) ? 0 : node(row, pn);
	if (n)
	    return createIndex(row, column, n);
    }
    return QModelIndex::Null;
}

QModelIndex Model::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        Node *n = static_cast<Node*>(child.data());
        Node *p = parent(n);
        if (p)
            return createIndex(row(p), 0, p);
    }
    return QModelIndex::Null;
}

int Model::rowCount(const QModelIndex &parent) const
{
    Node *n = static_cast<Node*>(parent.data());
    if (n && depth(n) >= d)
        return 0;
    return rc;
}

int Model::columnCount(const QModelIndex &parent) const
{
    Node *n = static_cast<Node*>(parent.data());
    if (n && depth(n) >= d)
        return 0;
    return cc;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    static QIconSet folder(QPixmap("folder.png"));

    if (role == DisplayRole)
	return "Item " + QAbstractItemModelPrivate::i2s(strbuf, 65, index.row()) + ":"
            + QAbstractItemModelPrivate::i2s(strbuf, 65, index.column());
    if (role == DecorationRole)
	return folder;
    return QVariant();
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QIconSet service(QPixmap("services.png"));

    if (orientation == Qt::Vertical) {
        if (role == DisplayRole)
            return QAbstractItemModelPrivate::i2s(strbuf, 65, section);
        if (role == DecorationRole)
            return service;
        return QVariant();
    } else {
        if (role == DisplayRole)
            return QAbstractItemModelPrivate::i2s(strbuf, 65, section);
        if (role == DecorationRole)
            return service;
        return QVariant();
    }

    return QVariant();
}

bool Model::hasChildren(const QModelIndex &parent) const
{
    Node *n = static_cast<Node*>(parent.data());
    if (n && (depth(n) >= d || parent.column() > 0))
        return false;
    return rc > 0 && cc > 0;
}

QAbstractItemModel::ItemFlags Model::flags(const QModelIndex &) const
{
    return (QAbstractItemModel::ItemIsDragEnabled
            |QAbstractItemModel::ItemIsSelectable
            |QAbstractItemModel::ItemIsEnabled);
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

int Model::depth(Node *node) const
{
    if (!node)
        return -1;
    int result = 0;
    while (node->parent && ++result)
        node = node->parent;
    return result;
}
