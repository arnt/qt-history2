#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qmap.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlFieldList;
class Q_EXPORT QSqlResultField
{
    friend class QSqlFieldList;
public:
    QSqlResultField( const QString& fieldName = QString::null, int fieldNumber = -1, QVariant::Type type = QVariant::Invalid );
    QSqlResultField( const QSqlResultField& other );
    QSqlResultField& operator=( const QSqlResultField& other );
    virtual bool operator==( const QSqlResultField& other );
    virtual ~QSqlResultField();

    QVariant      value();
    void          setValue( const QVariant& value );

    void          setName( const QString& name ) { nm = name; }
    QString       name() const { return nm; }
    void          setFieldNumber( int fieldNumber ) { num = fieldNumber;}
    int           fieldNumber() const { return num; }
    QVariant::Type type() const { return val.type(); }

private:
    QString       nm;
    int           num;
    QVariant      val;
};

class Q_EXPORT QSqlField : public QSqlResultField
{
public:
    QSqlField( const QString& fieldName = QString::null, int fieldNumber = -1, QVariant::Type type = QVariant::Invalid );
    QSqlField( const QSqlField& other );
    QSqlField& operator=( const QSqlField& other );
    bool operator==(const QSqlField& other) const;
    ~QSqlField();

    void          setDisplayLabel( const QString& l ) { label = l; }
    QString       displayLabel() const { return label; }
    void          setReadOnly( bool readOnly ) { ro = readOnly; }
    bool          isReadOnly() const { return ro; }
    void          setIsNull( bool n ) { nul = n; }
    bool          isNull() const { return nul; }
    void          setPrimaryIndex( bool primaryIndex ) { pIdx = primaryIndex; }
    bool          isPrimaryIndex() const { return pIdx; }
    void          setIsVisible( bool visible ) { iv = visible; }
    bool          isVisible() const { return iv; }
    void          setCalculated( bool calculated ) { cf = calculated; }
    bool          isCalculated() const { return cf; }
    
private:
    QString       label;
    bool          ro;
    bool          nul;
    bool          pIdx;
    bool          iv;
    bool          cf;    
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QValueList< QSqlField >;
template class Q_EXPORT QMap< QString, int >;
// MOC_SKIP_END
#endif

class Q_EXPORT QSqlFieldList
{
public:
    QSqlFieldList();
    QSqlFieldList( const QSqlFieldList& other );
    QSqlFieldList& operator=( const QSqlFieldList& other );
    QSqlFieldList( const QSqlField& t );
    virtual ~QSqlFieldList();
    QVariant&            operator[]( int i );
    QVariant&            operator[]( const QString& name );
    QVariant             value( int i );
    QVariant             value( const QString& name );
    int                  position( const QString& name ) const;
    QSqlField&           field( int i );
    const QSqlField&     field( int i ) const;
    QSqlField&           field( const QString& name );
    const QSqlField&     field( const QString& name ) const;
    virtual void         append( const QSqlField& field );
    void                 clear();
    uint                 count() const;
    virtual QString      toString( const QString& prefix = QString::null ) const;
    
private:
    QSqlField&           findField( int i );
    QSqlField&           findField( const QString& name );
    QValueList< QSqlField > fieldList;
    QString              fieldListStr;
    QMap< QString, int > posMap;
};

#endif	// QT_NO_SQL
#endif
