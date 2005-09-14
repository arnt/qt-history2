#ifndef QTESTTABLE_H
#define QTESTTABLE_H

#include "QtTest/qtest_global.h"

class QTestData;
class QTestTablePrivate;

class Q_TESTLIB_EXPORT QTestTable
{
public:
    enum Type { GlobalTable, LocalTable };
    QTestTable(Type t = LocalTable);
    ~QTestTable();

    void defineElement(const char *elementType, const char *elementName);
    QTestData *newData(const char *tag);

    int elementCount() const;
    int dataCount() const;

    int elementTypeId(int index) const;
    const char *dataTag(int index) const;
    const char *elementType(int index) const;
    int indexOf(const char *elementName) const;
    bool isEmpty() const;
    QTestData *testData(int index) const;

    static QTestTable *globalTestTable();
    static QTestTable *currentTestTable();

private:
    Q_DISABLE_COPY(QTestTable)

    QTestTablePrivate *d;
};

#endif
