#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#ifndef QT_H
#include "qnamespace.h"
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qmap.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlField
{
    friend class QSqlFieldList;
public:
    QSqlField( const QString& fieldName = QString::null, int fieldNumber = -1, QVariant::Type type = QVariant::Invalid );
    QSqlField( const QSqlField& other );
    QSqlField& operator=( const QSqlField& other );
    bool operator==(const QSqlField& other) const;
    ~QSqlField();

    QVariant           value() const;
    void               setValue( const QVariant& value );

    void               setName( const QString& name ) { nm = name; }
    QString            name() const { return nm; }
    void               setFieldNumber( int fieldNumber ) { num = fieldNumber;}
    int                fieldNumber() const { return num; }
    QVariant::Type     type() const { return val.type(); }

    void               setDisplayLabel( const QString& l ) { label = l; }
    QString            displayLabel() const { return label; }
    void               setReadOnly( bool readOnly ) { ro = readOnly; }
    bool               isReadOnly() const { return ro; }
    void               setIsNull( bool n ) { nul = n; }
    bool               isNull() const { return nul; }
    void               setPrimaryIndex( bool primaryIndex ) { pIdx = primaryIndex; }
    bool               isPrimaryIndex() const { return pIdx; }
    void               setIsVisible( bool visible ) { iv = visible; }
    bool               isVisible() const { return iv; }
    void               setCalculated( bool calculated ) { cf = calculated; }
    bool               isCalculated() const { return cf; }
    void               setAlignment( Qt::AlignmentFlags align ) { af = align; }
    Qt::AlignmentFlags alignment() const { return af; }

private:
    QString       nm;
    int           num;
    QVariant      val;
    QString       label;
    bool          ro;
    bool          nul;
    bool          pIdx;
    bool          iv;
    bool          cf;
    Qt::AlignmentFlags af;
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
    //    QSqlFieldList( const QSqlField& t );
    virtual ~QSqlFieldList();
    QVariant&            operator[]( int i );
    QVariant&            operator[]( const QString& name );
    virtual QVariant     value( int i );
    virtual QVariant     value( const QString& name );
    virtual void         setValue( int i, const QVariant& val );
    virtual void         setValue( const QString& name, const QVariant& val );
    int                  position( const QString& name ) const;
    QSqlField*           field( int i );
    const QSqlField*     field( int i ) const;
    QSqlField*           field( const QString& name );
    const QSqlField*     field( const QString& name ) const;

    virtual void         append( const QSqlField* field );
    virtual void         prepend( const QSqlField* field );
    virtual void         insert( int pos, const QSqlField* field );
    virtual void         remove( int pos );

    bool                 isEmpty() const { return fieldList.isEmpty(); }
    void                 clear();
    void                 clearValues();
    uint                 count() const;
    virtual QString      toString( const QString& prefix = QString::null ) const;

private:
    QSqlField*           findField( int i );
    QSqlField*           findField( const QString& name );
    QValueList< QSqlField > fieldList;
};

#endif	// QT_NO_SQL
#endif
