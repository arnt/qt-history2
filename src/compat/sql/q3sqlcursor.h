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

#ifndef Q3SQLCURSOR_H
#define Q3SQLCURSOR_H

#include "qcorevariant.h"
#include "qsqldatabase.h"
#include "qsqlrecord.h"
#include "qstringlist.h"
#include "qsqlquery.h"
#include "qsqlindex.h"

#ifndef QT_NO_SQL

class Q3SqlCursorPrivate;
class Q3SqlFieldInfo;

class Q_COMPAT_EXPORT Q3SqlCursor : public QSqlRecord, public QSqlQuery
{
public:
    Q3SqlCursor(const QString & name = QString(), bool autopopulate = true,
               QSqlDatabase db = QSqlDatabase());
    Q3SqlCursor(const Q3SqlCursor & other);
    Q3SqlCursor& operator=(const Q3SqlCursor& other);
    ~Q3SqlCursor();

    enum Mode {
        ReadOnly = 0,
        Insert = 1,
        Update = 2,
        Delete = 4,
        Writable = 7
    };

    QCoreVariant value(int i) const;
    inline QCoreVariant value(const QString &name) const { return value(indexOf(name)); }
    virtual void setValue(int i, const QCoreVariant &val);
    inline void setValue(const QString &name, const QCoreVariant &val) { setValue(indexOf(name), val); }
    virtual QSqlIndex primaryIndex(bool prime = true) const;
    virtual QSqlIndex index(const QStringList& fieldNames) const;
    QSqlIndex index(const QString& fieldName) const;
    virtual void setPrimaryIndex(const QSqlIndex& idx);

    virtual void append(const Q3SqlFieldInfo& fieldInfo);
    virtual void insert(int pos, const Q3SqlFieldInfo &fieldInfo);
    virtual void remove(int pos);
    virtual void clear();
    virtual void setGenerated(const QString& name, bool generated);
    virtual void setGenerated(int i, bool generated);

    virtual QSqlRecord*        editBuffer(bool copy = false);
    virtual QSqlRecord*        primeInsert();
    virtual QSqlRecord*        primeUpdate();
    virtual QSqlRecord*        primeDelete();
    virtual int                insert(bool invalidate = true);
    virtual int                update(bool invalidate = true);
    virtual int                del(bool invalidate = true);

    virtual void        setMode(int flags);
    int                        mode() const;
    virtual void        setCalculated(const QString& name, bool calculated);
    bool                isCalculated(const QString& name) const;
    virtual void        setTrimmed(const QString& name, bool trim);
    bool                isTrimmed(const QString& name) const;

    bool                isReadOnly() const;
    bool                canInsert() const;
    bool                canUpdate() const;
    bool                canDelete() const;

    bool                select();
    bool                select(const QSqlIndex& sort);
    bool                select(const QSqlIndex & filter, const QSqlIndex & sort);
    virtual bool        select(const QString & filter, const QSqlIndex & sort = QSqlIndex());

    virtual void        setSort(const QSqlIndex& sort);
    QSqlIndex                sort() const;
    virtual void        setFilter(const QString& filter);
    QString                filter() const;
    virtual void        setName(const QString& name, bool autopopulate = true);
    QString                name() const;
    QString                toString(const QString& prefix = QString(),
                                const QString& sep = ",") const;
    bool                 isNull(int i) const;
    bool                 isNull(const QString& name) const;

protected:
    void                afterSeek();
    bool                exec(const QString & sql);

    virtual QCoreVariant calculateField(const QString& name);
    virtual int                update(const QString & filter, bool invalidate = true);
    virtual int                del(const QString & filter, bool invalidate = true);

    virtual QString        toString(const QString& prefix, QSqlField* field, const QString& fieldSep) const;
    virtual QString        toString(QSqlRecord* rec, const QString& prefix, const QString& fieldSep,
                                const QString& sep) const;
    virtual QString        toString(const QSqlIndex& i, QSqlRecord* rec, const QString& prefix,
                                const QString& fieldSep, const QString& sep) const;

private:
    void                sync();
    int                        apply(const QString& q, bool invalidate);
    int                        applyPrepared(const QString& q, bool invalidate);
    QSqlRecord&                operator=(const QSqlRecord & list);
    void                 append(const QSqlField& field);

    Q3SqlCursorPrivate*        d;
};

#endif        // QT_NO_SQL

#endif
