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

#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#include "qstring.h"
#include "qsqlrecord.h"
#include "qlist.h"

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

class QSqlCursor;

class QM_EXPORT_SQL QSqlIndex : public QSqlRecord
{
public:
    QSqlIndex(const QString& cursorName = QString(), const QString& name = QString());
    QSqlIndex(const QSqlIndex& other);
    ~QSqlIndex();
    QSqlIndex&       operator=(const QSqlIndex& other);
    void             setCursorName(const QString& cursorName);
    QString          cursorName() const { return cursor; }
    void             setName(const QString& name);
    QString          name() const { return nm; }

    void             append(const QSqlField& field);
    void             append(const QSqlField& field, bool desc);

    bool             isDescending(int i) const;
    void             setDescending(int i, bool desc);

    QString          toString(const QString& prefix = QString(),
                               const QString& sep = QLatin1String(","),
                               bool verbose = true) const;
    QStringList      toStringList(const QString& prefix = QString(),
                                   bool verbose = true) const;

private:
    QString          createField(int i, const QString& prefix, bool verbose) const;
    QString          cursor;
    QString          nm;
    QList<bool> sorts;
};

#endif        // QT_NO_SQL
#endif
