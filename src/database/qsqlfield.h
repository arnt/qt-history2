#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qmap.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlResultField
{
public:
    QSqlResultField( const QString& fieldName = QString::null, int fieldNumber = -1, QVariant::Type type = QVariant::Invalid );
    virtual ~QSqlResultField();

    QVariant&     value();

    void          setName( const QString& name ) { nm = name; }
    QString       name() const { return nm; }
    void          setFieldNumber( int fieldNumber ) { num = fieldNumber;}
    int           fieldNumber() const { return num; }
    QVariant::Type type() const { return val.type(); }

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const QSqlResultField& ) const { return FALSE; }
#endif

private:
    QVariant      val;
    QString       nm;
    int           num;
};

class QSqlField : public QSqlResultField
{
public:
    QSqlField( const QString& fieldName = QString::null, int fieldNumber = -1, QVariant::Type type = QVariant::Invalid );
    virtual ~QSqlField();

    void          setDisplayLabel( const QString& l ) { label = l; }
    QString       displayLabel() const { return label; }
    void          setReadOnly( bool readOnly ) { ro = readOnly; }
    bool          isReadOnly() const { return ro; }
    void          setIsNull( bool n ) { nul = n; }
    bool          isNull() const { return nul; }
    void          setPrimaryIndex( bool primaryIndex ) { pIdx = primaryIndex; }
    bool          isPrimaryIndex() const { return pIdx; }

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const QSqlField& ) const { return FALSE; }
#endif

private:
    QString       label;
    bool          ro;
    bool          nul;
    bool          pIdx;
};

template< class T >
class QSqlFields
{
public:
    QSqlFields() {}
    QSqlFields ( const QSqlFields<T>& l )
    {
	fieldList = l.fieldList;
	fieldListStr = l.fieldListStr;
	posMap = l.posMap;
    }
    virtual ~QSqlFields() {}
    QVariant& operator[]( int i ) { return value( i ); }
    QVariant& operator[]( const QString& name ) { return value( name ); }
    QVariant& value( int i )
    {
#ifdef CHECK_RANGE
	static QVariant dbg;
	if( (unsigned int) i > fieldList.count() ){
	    qWarning( "QSqlFields warning: index out of range" );
	    return dbg;
	}
#endif // CHECK_RANGE
	return fieldList[ i ].value();
    }
    QVariant& value( const QString& name )
    {
#ifdef CHECK_RANGE
	static QVariant dbg;
	if( (unsigned int) position( name ) > fieldList.count() ){
	    qWarning( "QSqlFields warning: index out of range" );
	    return dbg;
	}
#endif // CHECK_RANGE
	return fieldList[ position( name ) ].value();
    }
    T& field( int i ) { return fieldList[ i ]; }
    T& field( const QString& name ) { return fieldList[ position( name ) ]; }
    int position( const QString& name )
    {
	if ( posMap.contains( name ) )
	    return posMap[ name ];
	return -1;
    }
    void append( const T& field )
    {
	if ( fieldListStr.isNull() )
	    fieldListStr = field.name();
	else
	    fieldListStr += ", " + field.name();
	posMap[ field.name() ] = fieldList.count();
	fieldList.append( field );
    }
    void clear()
    {
	fieldListStr = QString::null;
	fieldList.clear();
	posMap.clear();
    }
    uint count() const { return fieldList.count(); }
    QString toString() const { return fieldListStr; }
private:
    QString fieldListStr;
    QMap< QString, int > posMap;
    QValueList< T > fieldList;
};

typedef QSqlFields< QSqlField > QSqlFieldList;

#endif	// QT_NO_SQL
#endif
