/****************************************************************************
**
** Definition of QSqlRecord private class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_NO_SQL

#ifndef QSQLRECORD_P_H
#define QSQLRECORD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#include "qatomic.h"
#include "qsqlfield.h"
#include "qstring.h"
#include "qvector.h"

class QSqlRecordPrivate
{
public:
    QSqlRecordPrivate();
    QSqlRecordPrivate(const QSqlRecordPrivate &other);
    virtual ~QSqlRecordPrivate();

    virtual QString toString() const;
    virtual QSqlRecordPrivate *clone() const;

    inline bool contains(int i) { return i >= 0 && i < fields.count(); }

    QString createField(int i, const QString &prefix) const;

    QVector<QSqlField> fields;
    QAtomic ref;
};

#endif //QSQLRECORD_P_H
#endif //QT_NO_SQL
