#include "qmap.h"

QMapData QMapData::shared_null = {
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
    reinterpret_cast<Node *>(&shared_null),
#endif
    { reinterpret_cast<Node *>(&shared_null), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, Q_ATOMIC_INIT(1), 0,
    0, 0, false
};

QMapData *QMapData::createData()
{
    QMapData *d = new QMapData;
    Node *e = reinterpret_cast<Node *>(d);
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
    d->backward = e;
#endif
    d->forward[0] = e;
    d->ref = 1;
    d->topLevel = 0;
    d->size = 0;
    d->randomBits = 0;
    d->insertInOrder = false;
    return d;
}

void QMapData::continueFreeData(int offset)
{
    Node *e = reinterpret_cast<Node *>(this);
    Node *cur = forward[0];
    Node *prev;
    while (cur != e) {
	prev = cur;
	cur = cur->forward[0];
	qFree(reinterpret_cast<char *>(prev) - offset);
    }
    delete this;
}

QMapData::Node *QMapData::node_create(Node *update[], int offset)
{
    int level = 0;
    uint mask = (1 << Sparseness) - 1;

    while ((randomBits & mask) == mask && level < LastLevel) {
	++level;
	mask <<= Sparseness;
    }

    ++randomBits;
    if (level == 3 && !insertInOrder)
	randomBits = qRand();

    if (level > topLevel) {
	Node *e = reinterpret_cast<Node *>(this);
	level = ++topLevel;
	e->forward[level] = e;
	update[level] = e;
    }

    void *concreteNode = qMalloc(offset + sizeof(Node) + level * sizeof(Node *));
    Node *abstractNode = reinterpret_cast<Node *>(reinterpret_cast<char *>(concreteNode) + offset);

#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
    abstractNode->backward = update[0];
    update[0]->forward[0]->backward = abstractNode;
#endif

    for (int i = level; i >= 0; i--) {
	abstractNode->forward[i] = update[i]->forward[i];
	update[i]->forward[i] = abstractNode;
	update[i] = abstractNode;
    }
    ++size;
    return abstractNode;
}

void QMapData::node_delete(Node *update[], int offset, Node *node)
{
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
    node->forward[0]->backward = node->backward;
#endif

    for (int i = 0; i <= topLevel; ++i) {
	if (update[i]->forward[i] != node)
	    break;
	update[i]->forward[i] = node->forward[i];
    }
    --size;
    qFree(reinterpret_cast<char *>(node) - offset);
}

#ifndef QT_NO_DEBUG
#include <qstring.h>
#include <qvector.h>

uint QMapData::adjust_ptr(Node *node)
{
    if (node == reinterpret_cast<Node *>(this)) {
	return (uint)0xFFFFFFFF;
    } else {
	return (uint)node;
    }
}

void QMapData::dump()
{
    qDebug("Map data (ref = %d, size = %d, randomBits = %#.8x)", ref.atomic, size, randomBits);

    QString preOutput;
    QVector<QString> output(topLevel + 1);
    Node *e = reinterpret_cast<Node *>(this);

    QString str;
    str.sprintf("    %.8x", adjust_ptr(reinterpret_cast<Node *>(this)));
    preOutput += str;

    Node *update[LastLevel + 1];
    for (int i = 0; i <= topLevel; ++i) {
        str.sprintf("%d: [%.8x] -", i, adjust_ptr(forward[i]));
        output[i] += str;
	update[i] = forward[i];
    }

    Node *node = forward[0];
    while (node != e) {
	int level = 0;
        while (level < topLevel && update[level + 1] == node)
	    ++level;

	str.sprintf("       %.8x", adjust_ptr(node));
	preOutput += str;

	for (int i = 0; i <= level; ++i) {
            str.sprintf("-> [%.8x] -", adjust_ptr(node->forward[i]));
	    output[i] += str;
	    update[i] = node->forward[i];
        }
        for (int j = level + 1; j <= topLevel; ++j)
	    output[j] += "---------------";
	node = node->forward[0];
    }

    qDebug(preOutput.ascii());
    for (int i = 0; i <= topLevel; ++i)
	qDebug(output[i].ascii());
}
#endif
