#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlField
{
public:
    QSqlField( const QString& fieldName = QString::null, int fieldNumber = -1, QVariant::Type type = QVariant::Invalid );
    virtual ~QSqlField();

    QVariant      value() const;
    void          setValue( const QVariant& v );

    void          setName( const QString& name ) { nm = name; }
    QString       name() const { return nm; }
    void          setDisplayLabel( const QString& l ) { label = l; }
    QString       displayLabel() const { return label; }
    void          setFieldNumber( int fieldNumber ) { num = fieldNumber;}
    int           fieldNumber() const { return num; }
    void          setReadOnly( bool readOnly ) { ro = readOnly; }
    bool          isReadOnly() const { return ro; }
    void          setIsNull( bool n ) { nul = n; }
    bool          isNull() const { return nul; }
    QVariant::Type type() const { return val.type(); }

private:
    QVariant      val;
    QString       nm;
    int           num;
    QString       label;
    bool          ro;
    bool          nul;

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const QSqlField& ) const { return FALSE; }
#endif
};

class QSqlFieldList : public QValueList<QSqlField>
{
public:
    QSqlFieldList();
    QSqlFieldList ( const QSqlFieldList& l );
    virtual ~QSqlFieldList();
    QSqlField& operator[]( int i );
    QSqlField& operator[]( const QString& name );
    int position( const QString& name );
};

#endif	// QT_NO_SQL
#endif
