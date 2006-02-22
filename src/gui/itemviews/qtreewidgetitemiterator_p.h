#ifndef QTREEWIDGETITEMITERATOR_P_H
#define QTREEWIDGETITEMITERATOR_P_H
#include <QtCore/QStack>
#include "qtreewidgetitemiterator.h"

class QTreeModel;
class QTreeWidgetItem;

class QTreeWidgetItemIteratorPrivate {
    Q_DECLARE_PUBLIC(QTreeWidgetItemIterator)
public:
    QTreeWidgetItemIteratorPrivate(QTreeWidgetItemIterator *q, QTreeModel *model)
        : m_currentIndex(0), m_model(model), q_ptr(q)
    {

    }

    QTreeWidgetItemIteratorPrivate(const QTreeWidgetItemIteratorPrivate& other)
        : m_currentIndex(other.m_currentIndex), m_model(other.m_model), m_parentIndex(other.m_parentIndex)
    {

    }

    QTreeWidgetItemIteratorPrivate &operator=(const QTreeWidgetItemIteratorPrivate& other)
    {
        m_currentIndex = other.m_currentIndex;
        m_parentIndex = other.m_parentIndex;
        m_model = other.m_model;
        return (*this);
    }

    ~QTreeWidgetItemIteratorPrivate()
    {
    }

    QTreeWidgetItem* nextSibling(const QTreeWidgetItem* item) const;
    void ensureValidIterator(const QTreeWidgetItem *itemToBeRemoved);

    QTreeWidgetItem *next(const QTreeWidgetItem *current);
    QTreeWidgetItem *previous(const QTreeWidgetItem *current);
private:
    int             m_currentIndex;
    QTreeModel     *m_model;        // This iterator class should not have ownership of the model.
    QStack<int>     m_parentIndex;
    QTreeWidgetItemIterator *q_ptr;
};

#endif //QTREEWIDGETITEMITERATOR_P_H
