#ifndef QTESTTABLE_H
#define QTESTTABLE_H

#include "QtTest/qttest_global.h"

class QtTestData;
class QtTestTablePrivate;

class Q_TESTLIB_EXPORT QtTestTable
{
public:
    QtTestTable();
    ~QtTestTable();

    void defineElement(const char *elementType, const char *elementName);
    QtTestData *newData(const char *tag);

    int elementCount() const;
    int dataCount() const;

    int elementTypeId(int index) const;
    const char *dataTag(int index) const;
    const char *elementType(int index) const;
    int indexOf(const char *elementName) const;
    bool isEmpty() const;
    QtTestData *testData(int index) const;

    static QtTestTable *globalTestTable();

private:
    Q_DISABLE_COPY(QtTestTable)

    QtTestTablePrivate *d;
};

#endif
