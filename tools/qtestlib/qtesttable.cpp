#include "QtTest/qtesttable.h"
#include "QtTest/qtestdata.h"

#include <QtCore/qmetaobject.h>

#include <string.h>

#include "QtTest/qtestassert.h"

class QTestTablePrivate
{
public:
    struct ElementList
    {
        ElementList(): elementName(0), elementType(0), next(0) {}
        const char *elementName;
        const char *elementType;
        ElementList *next;
    };

    struct DataList
    {
        DataList(): data(0), next(0) {}
        QTestData *data;
        DataList *next;
    };

    QTestTablePrivate(): list(0), dataList(0), type(QTestTable::LocalTable) {}
    ~QTestTablePrivate();

    ElementList *list;
    DataList *dataList;
    QTestTable::Type type;

    void append(const char *elemType, const char *elemName);
    void append(QTestData *data);
    ElementList *elementAt(int index);
    QTestData *dataAt(int index);

    static QTestTable *currentTestTable;
};

QTestTable *QTestTablePrivate::currentTestTable = 0;

QTestTablePrivate::ElementList *QTestTablePrivate::elementAt(int index)
{
    ElementList *iter = list;
    for (int i = 0; i < index; ++i) {
        if (!iter)
            return 0;
        iter = iter->next;
    }
    return iter;
}

QTestData *QTestTablePrivate::dataAt(int index)
{
    DataList *iter = dataList;
    for (int i = 0; i < index; ++i) {
        if (!iter)
            return 0;
        iter = iter->next;
    }
    return iter ? iter->data : 0;
}

QTestTablePrivate::~QTestTablePrivate()
{
    DataList *dit = dataList;
    while (dit) {
        DataList *next = dit->next;
        delete dit->data;
        delete dit;
        dit = next;
    }
    ElementList *iter = list;
    while (iter) {
        ElementList *next = iter->next;
        delete iter;
        iter = next;
    }
}

void QTestTablePrivate::append(const char *elemType, const char *elemName)
{
    ElementList *item = new ElementList;
    item->elementName = elemName;
    item->elementType = elemType;
    if (!list) {
        list = item;
        return;
    }
    ElementList *last = list;
    while (last->next != 0)
        last = last->next;
    last->next = item;
}

void QTestTablePrivate::append(QTestData *data)
{
    DataList *item = new DataList;
    item->data = data;
    if (!dataList) {
        dataList = item;
        return;
    }
    DataList *last = dataList;
    while (last->next != 0)
        last = last->next;
    last->next = item;
}

void QTestTable::defineElement(const char *elementType, const char *elementName)
{
    QTEST_ASSERT(elementType);
    QTEST_ASSERT(elementName);

    d->append(elementType, elementName);
}

int QTestTable::elementCount() const
{
    QTestTablePrivate::ElementList *item = d->list;
    int count = 0;
    while (item) {
        ++count;
        item = item->next;
    }
    return count;
}


int QTestTable::dataCount() const
{
    QTestTablePrivate::DataList *item = d->dataList;
    int count = 0;
    while (item) {
        ++count;
        item = item->next;
    }
    return count;
}

bool QTestTable::isEmpty() const
{
    return !d->list;
}

QTestData *QTestTable::newData(const char *tag)
{
    QTestData *dt = new QTestData(tag, this);
    d->append(dt);
    return dt;
}

QTestTable::QTestTable(Type t)
{
    d = new QTestTablePrivate;
    d->type = t;
    if (t == LocalTable)
        QTestTablePrivate::currentTestTable = this;
}

QTestTable::~QTestTable()
{
    if (d->type == LocalTable)
        QTestTablePrivate::currentTestTable = 0;
    delete d;
}

int QTestTable::elementTypeId(int index) const
{
    QTestTablePrivate::ElementList *item = d->elementAt(index);
    if (!item)
        return -1;
    return QMetaType::type(item->elementType);
}

const char *QTestTable::dataTag(int index) const
{
    QTestTablePrivate::ElementList *item = d->elementAt(index);
    if (!item)
        return 0;
    return item->elementName;
}

const char *QTestTable::elementType(int index) const
{
    QTestTablePrivate::ElementList *item = d->elementAt(index);
    if (!item)
        return 0;
    return item->elementType;
}

QTestData *QTestTable::testData(int index) const
{
    return d->dataAt(index);
}

int QTestTable::indexOf(const char *elementName) const
{
    QTEST_ASSERT(elementName);

    QTestTablePrivate::ElementList *item = d->list;
    int i = 0;
    while (item) {
        if (strcmp(elementName, item->elementName) == 0)
            return i;
        item = item->next;
        ++i;
    }
    return -1;
}

QTestTable *QTestTable::globalTestTable()
{
    static QTestTable *gTable = new QTestTable(GlobalTable);
    return gTable;
}

QTestTable *QTestTable::currentTestTable()
{
    return QTestTablePrivate::currentTestTable;
}

