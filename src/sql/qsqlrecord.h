#ifndef QSQLRECORD_H
#define QSQLRECORD_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qmap.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QValueList< QSqlField >;
template class Q_EXPORT QMap< QString, int >;
// MOC_SKIP_END
#endif

class Q_EXPORT QSqlRecord
{
public:
    QSqlRecord();
    QSqlRecord( const QSqlRecord& other );
    QSqlRecord& operator=( const QSqlRecord& other );
    virtual ~QSqlRecord();
    virtual QVariant     value( int i );
    virtual QVariant     value( const QString& name );
    virtual void         setValue( int i, const QVariant& val );
    virtual void         setValue( const QString& name, const QVariant& val );
    int                  position( const QString& name ) const;
    QSqlField*           field( int i );
    const QSqlField*     field( int i ) const;
    QSqlField*           field( const QString& name );
    const QSqlField*     field( const QString& name ) const;

    virtual void         append( const QSqlField& field );
    virtual void         prepend( const QSqlField& field );
    virtual void         insert( int pos, const QSqlField& field );
    virtual void         remove( int pos );

    bool                 isEmpty() const { return fieldList.isEmpty(); }
    void                 clear();
    void                 clearValues( bool nullify = FALSE );
    uint                 count() const;
    virtual QString      toString( const QString& prefix = QString::null ) const;

private:
    QSqlField*           findField( int i );
    QSqlField*           findField( const QString& name );
    QValueList< QSqlField > fieldList;
};

#endif	// QT_NO_SQL
#endif
