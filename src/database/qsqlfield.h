#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qmap.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlResultField
{
public:
    QSqlResultField( const QString& fieldName = QString::null, int fieldNumber = -1, QVariant::Type type = QVariant::Invalid );
    QSqlResultField( const QSqlResultField& other );
    QSqlResultField& operator=( const QSqlResultField& other );
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

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const QSqlField& ) const { return FALSE; }
#endif
private:
    QString       label;
    bool          ro;
    bool          nul;
    bool          pIdx;
    bool          iv;
    bool          cf;
};

template< class T >
class Q_EXPORT QSqlFields
{
public:
    QSqlFields() {}
    QSqlFields( const QSqlFields<T>& other )
	: fieldList( other.fieldList ), fieldListStr( other.fieldListStr ), posMap( other.posMap )
    {
    }
    QSqlFields< T >& operator=( const QSqlFields<T>& other )
    {
	fieldList = other.fieldList;
	fieldListStr = other.fieldListStr;
	posMap = other.posMap;
	return *this;
    }
    QSqlFields( const T& t )
    {
	append( t );
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
    int position( const QString& name ) const
    {
	if ( posMap.contains( name ) )
	    return posMap[ name ];
	return -1;
    }
    T& field( int i ) { return fieldList[ i ]; }
    const T& field( int i ) const { return fieldList[ i ]; }
    T& field( const QString& name ) { return fieldList[ position( name ) ]; }
    const T& field( const QString& name ) const { return fieldList[ position( name ) ]; }
    virtual void append( const T& field )
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
    virtual QString toString( const QString& prefix = QString::null ) const
    {
	if ( prefix.isNull() )
	    return fieldListStr;
	QString pfix =  prefix + ".";
	QString pflist = fieldListStr;
	pflist = pfix + pflist.replace( QRegExp(", "), QString(", ") + pfix );
	return pflist;
    }
private:
    QValueList< T > fieldList;
    QString fieldListStr;
    QMap< QString, int > posMap;
};

typedef QSqlFields< QSqlField > QSqlFieldList;

#endif	// QT_NO_SQL
#endif
