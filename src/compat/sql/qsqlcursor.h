/****************************************************************************
**
** Definition of QSqlCursor class.
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

#ifndef QSQLCURSOR_H
#define QSQLCURSOR_H

#ifndef QT_H
#include "qcorevariant.h"
#include "qsqldatabase.h"
#include "qsqlrecord.h"
#include "qstringlist.h"
#include "qsqlquery.h"
#include "qsqlindex.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlCursorPrivate;
class QSqlFieldInfo;

class Q_COMPAT_EXPORT QSqlCursor : public QSqlRecord, public QSqlQuery
{
public:
    QSqlCursor(const QString & name = QString(), bool autopopulate = true,
               QSqlDatabase db = QSqlDatabase());
    QSqlCursor(const QSqlCursor & other);
    QSqlCursor& operator=(const QSqlCursor& other);
    ~QSqlCursor();

    enum Mode {
        ReadOnly = 0,
        Insert = 1,
        Update = 2,
        Delete = 4,
        Writable = 7
    };

    QCoreVariant value(int i) const;
    inline QCoreVariant value(const QString &name) const { return value(indexOf(name)); }
    void setValue(int i, const QCoreVariant &val);
    inline void setValue(const QString &name, const QCoreVariant &val) { setValue(indexOf(name), val); }
    virtual QSqlIndex primaryIndex(bool prime = true) const;
    virtual QSqlIndex index(const QStringList& fieldNames) const;
    QSqlIndex index(const QString& fieldName) const;
    virtual void setPrimaryIndex(const QSqlIndex& idx);

    virtual void append(const QSqlFieldInfo& fieldInfo);
#ifdef QT_COMPAT
    inline QT_COMPAT void insert(int pos, const QSqlFieldInfo &fieldInfo) { replace(pos, fieldInfo); }
#endif
    virtual void replace(int pos, const QSqlFieldInfo &fieldInfo);
    void remove(int pos);
    void clear();
    void setGenerated(const QString& name, bool generated);
    void setGenerated(int i, bool generated);

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

    QSqlCursorPrivate*        d;
};




#endif        // QT_NO_SQL
#endif
