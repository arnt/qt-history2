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

#ifndef QTESTDATA_H
#define QTESTDATA_H

#include <QtCore/qmetatype.h>
#include <QtCore/qstring.h>

#include "QtTest/qtest_global.h"

class QTestTable;
class QTestDataPrivate;

class Q_TESTLIB_EXPORT QTestData
{
public:
    ~QTestData();

    void append(int type, const void *data);
    void *data(int index) const;
    const char *dataTag() const;
    QTestTable *parent() const;

private:
    friend class QTestTable;
    QTestData(const char *tag = 0, QTestTable *parent = 0);

    Q_DISABLE_COPY(QTestData)

    QTestDataPrivate *d;
};

template<typename T>
QTestData &operator<<(QTestData &data, const T &value)
{
    data.append(QMetaTypeId<T>::qt_metatype_id(), &value);
    return data;
}

inline QTestData &operator<<(QTestData &data, const char * value)
{
    QString str = QString::fromAscii(value);
    data.append(QMetaType::QString, &str);
    return data;
}

#endif

