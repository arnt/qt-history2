#include <QtCore/qmetaobject.h>

#include "QTest/qtestassert.h"
#include "QTest/qtestdata.h"
#include "QTest/qtesttable.h"

#include <string.h>
#include <stdlib.h>

class QTestDataPrivate
{
public:
    QTestDataPrivate(): tag(0), parent(0), data(0), dataCount(0) {}

    char *tag;
    QTestTable *parent;
    void **data;
    int dataCount;
};

QTestData::QTestData(const char *tag, QTestTable *parent)
{
    QTEST_ASSERT(tag);
    QTEST_ASSERT(parent);
    d = new QTestDataPrivate;
    d->tag = strdup(tag);
    d->parent = parent;
    d->data = new void *[parent->elementCount()];
    memset(d->data, 0, parent->elementCount() * sizeof(void*));
}

QTestData::~QTestData()
{
    for (int i = 0; i < d->dataCount; ++i) {
        if (d->data[i])
            QMetaType::destroy(d->parent->elementTypeId(i), d->data[i]);
    }
    delete [] d->data;
    free(d->tag);
    delete d;
}

void QTestData::append(int type, const void *data)
{
    QTEST_ASSERT(d->dataCount < d->parent->elementCount());
    if (d->parent->elementTypeId(d->dataCount) != type) {
        qDebug("expected data of type '%s', got '%s' for element %d of data with tag '%s'",
                d->parent->elementType(d->dataCount), QMetaType::typeName(type),
                d->dataCount, d->tag);
        QTEST_ASSERT(false);
    }
    d->data[d->dataCount] = QMetaType::construct(type, data);
    ++d->dataCount;
}

const char *QTestData::expectedDataType() const
{
    QTEST_ASSERT(d->dataCount < d->parent->elementCount());
    return d->parent->elementType(d->dataCount);
}

void *QTestData::data(int index) const
{
    QTEST_ASSERT(index < d->parent->elementCount());
    return d->data[index];
}

QTestTable *QTestData::parent() const
{
    return d->parent;
}

const char *QTestData::dataTag() const
{
    return d->tag;
}

