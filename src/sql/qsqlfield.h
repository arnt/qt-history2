#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#ifndef QT_H
#include "qnamespace.h"
#include "qstring.h"
#include "qvariant.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlField
{
public:
    QSqlField( const QString& fieldName = QString::null, int fieldNumber = -1, QVariant::Type type = QVariant::Invalid );
    QSqlField( const QSqlField& other );
    QSqlField& operator=( const QSqlField& other );
    bool operator==(const QSqlField& other) const;
    ~QSqlField();

    QVariant           value() const;
    void               setValue( const QVariant& value );
    void               clear();

    void               setName( const QString& name ) { nm = name; }
    QString            name() const { return nm; }
    void               setFieldNumber( int fieldNumber ) { num = fieldNumber;}
    int                fieldNumber() const { return num; }
    QVariant::Type     type() const { return val.type(); }

    void               setDisplayLabel( const QString& l ) { label = l; }
    QString            displayLabel() const { return label; }
    void               setReadOnly( bool readOnly ) { ro = readOnly; }
    bool               isReadOnly() const { return ro; }
    void               setNull( bool n ) { nul = n; }
    bool               isNull() const { return nul; }
    void               setPrimaryIndex( bool primaryIndex ) { pIdx = primaryIndex; }
    bool               isPrimaryIndex() const { return pIdx; }
    void               setVisible( bool visible ) { iv = visible; }
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

#endif	// QT_NO_SQL
#endif
