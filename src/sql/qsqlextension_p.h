/****************************************************************************
**
** Definition of the QSqlExtension class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLEXTENSION_P_H
#define QSQLEXTENSION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qmap.h"
#include "qvaluevector.h"
#include "qstring.h"
#include "qvariant.h"
#include "qsql.h"
#endif // QT_H

#ifndef QT_NO_SQL

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#define QM_TEMPLATE_EXTERN_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#define QM_TEMPLATE_EXTERN_SQL Q_TEMPLATE_EXTERN
#endif

struct Param {
    Param( const QVariant& v = QVariant(), QSql::ParameterType t = QSql::In ): value( v ), typ( t ) {}
    QVariant value;
    QSql::ParameterType typ;
    Q_DUMMY_COMPARISON_OPERATOR(Param)
};

struct Holder {
    Holder( const QString& hldr = QString::null, int pos = -1 ): holderName( hldr ), holderPos( pos ) {}
    bool operator==( const Holder& h ) const { return h.holderPos == holderPos && h.holderName == holderName; }
    bool operator!=( const Holder& h ) const { return h.holderPos != holderPos || h.holderName != holderName; }
    QString holderName;
    int	    holderPos;
};

#define Q_DEFINED_QSQLEXTENSION
#include "qwinexport.h"

class QM_EXPORT_SQL QSqlExtension {
public:
    QSqlExtension();
    virtual ~QSqlExtension();
    virtual bool prepare( const QString& query );
    virtual bool exec();
    virtual void bindValue( const QString& holder, const QVariant& value, QSql::ParameterType = QSql::In );
    virtual void bindValue( int pos, const QVariant& value, QSql::ParameterType = QSql::In );
    virtual void addBindValue( const QVariant& value, QSql::ParameterType = QSql::In );
    virtual QVariant parameterValue( const QString& holder );
    virtual QVariant parameterValue( int pos );
    QVariant boundValue( const QString& holder ) const;
    QVariant boundValue( int pos ) const;
    QMap<QString, QVariant> boundValues() const;
    void clear();
    void clearValues();
    void clearIndex();
    void resetBindCount();

    enum BindMethod { BindByPosition, BindByName };
    BindMethod bindMethod(); // ### 4.0: make this const
    BindMethod bindm;
    int bindCount;

    QMap<int, QString> index;
    typedef QMap<QString, Param> ValueMap;
    ValueMap values;

    // convenience container for QSqlQuery
    // to map holders <-> positions
    typedef QVector<Holder> HolderVector;
    HolderVector holders;
};

class QM_EXPORT_SQL QSqlDriverExtension
{
public:
    QSqlDriverExtension();
    virtual ~QSqlDriverExtension();
    virtual bool isOpen() const = 0;
};

class QM_EXPORT_SQL QSqlOpenExtension
{
public:
    QSqlOpenExtension();
    virtual ~QSqlOpenExtension();
    virtual bool open( const QString& db,
		       const QString& user,
		       const QString& password,
		       const QString& host,
		       int port,
		       const QString& connOpts ) = 0;
};
#endif

#endif
