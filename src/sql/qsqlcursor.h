#ifndef QSQLCURSOR_H
#define QSQLCURSOR_H

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#include "qsqlfield.h"
#include "qsqlrecord.h"
#include "qsqlquery.h"
#include "qsqlindex.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDatabase;
class QSqlCursorPrivate;
class Q_EXPORT QSqlCursor : public QSqlRecord, public QSqlQuery
{
public:
    QSqlCursor( const QString & name = QString::null, bool autopopulate = TRUE, QSqlDatabase* db = 0 );
    QSqlCursor( const QSqlCursor & s );
    QSqlCursor& operator=( const QSqlCursor& s );
    ~QSqlCursor();
    
    enum Mode {
	ReadOnly = 0,
	Insert = 1,
	Update = 2,
	Delete = 4,
	Writable = 7
    };

    QVariant          value( int i );
    QVariant          value( const QString& name );
    QSqlIndex         primaryIndex( bool prime = FALSE ) const;
    QSqlIndex         index( const QStringList& fieldNames ) const;
    QSqlIndex         index( const QString& fieldName ) const;
    QSqlIndex         index( const char* fieldName ) const;
    void              setPrimaryIndex( QSqlIndex idx );

    virtual int       insert( bool invalidate = TRUE );
    virtual int       update( const QSqlIndex & filter = QSqlIndex(), bool invalidate = TRUE );
    virtual int       del( const QSqlIndex & filter = QSqlIndex(), bool invalidate = TRUE );

    void              setMode( int flags );
    int               mode() const;
    bool              isReadOnly() const;
    bool              canInsert() const;
    bool              canUpdate() const;
    bool              canDelete() const;
    void              detach();

    bool              select();
    bool              select( const QSqlIndex& sort );
    bool              select( const QSqlIndex & filter, const QSqlIndex & sort );
    bool              select( const QString & filter, const QSqlIndex & sort = QSqlIndex() );
    QSqlIndex         sort() const;
    QString           filter() const;
    void              setName( const QString& name, bool autopopulate = TRUE );
    QString           name() const;

protected:
    void              postSeek();

    QSqlRecord&       operator=( const QSqlRecord & list );
    bool              exec( const QString & str );
    QString           fieldEqualsValue( const QString& prefix, const QString& fieldSep, const QSqlIndex & i = QSqlIndex() );
    virtual QVariant  calculateField( uint fieldNumber );

    virtual int       update( const QString & filter, bool invalidate = TRUE );
    virtual int       del( const QString & filter, bool invalidate = TRUE );

private:
    QSqlRecord        fields() const;     //hide
    void              sync();
    int               apply( const QString& q, bool invalidate );
    QSqlCursorPrivate*  d;
};




#endif	// QT_NO_SQL
#endif
