#include "qmap.h"

QMapData QMapData::shared_null = {
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
    0,
#endif
    { (Node *)&shared_null, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, (Node *)&shared_null, Q_ATOMIC_INIT(1),
    -1, 0, 0
};

QMapData *QMapData::createData()
{
    QMapData *d = new QMapData;
    Node *e = (Node *)d;
#ifndef QT_NO_QMAP_BACKWARD_ITERATORS
    d->backward = e;
#endif
    d->forward[0] = e;
    d->cachedNode = e;
    d->ref = 1;
    d->topLevel = 0;
    d->size = 0;
    d->randomBits = 0;
    return d;
}

void QMapData::freeData(int offset)
{
    Node *e = (Node *)this;
    Node *cur = forward[0];
    Node *prev;
    while (cur != e) {
	prev = cur;
	cur = cur->forward[0];
	qFree((char *) prev - offset);
    }
    delete this;
}

QMapData::Node *QMapData::node_create(Node *update[], int offset)
{
    int level = 0;
    uint mask = 0x3;

    while ((randomBits & mask) == mask && level < LastLevel) {
	++level;
	mask <<= 2;
    }

    ++randomBits;
    if (level == 3)
	randomBits = qRand();

    if (level > topLevel) {
	Node *e = (Node *)this;
	level = ++topLevel;
	e->forward[level] = e;
	update[level] = e;
    }

    void *concreteNode = qMalloc(offset + sizeof(Node) + level * sizeof(Node *));
    Node *abstractNode = (Node *)((char *)concreteNode + offset);

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
    for (int i = 0; i <= topLevel; ++i) {
	if (update[i]->forward[i] != node)
	    break;
	update[i]->forward[i] = node->forward[i];
    }
    if (cachedNode == node)
	cachedNode = node->forward[0];
    --size;
    qFree((char *)node - offset);
}
