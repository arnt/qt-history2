/****************************************************************************
**
** Definition of QSqlField class.
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

#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#endif

#ifndef QT_NO_SQL

class QSqlFieldPrivate;

class QM_EXPORT_SQL QSqlField
{
public:
    QSqlField( const QString& fieldName = QString(), QVariant::Type type = QVariant::Invalid );
    QSqlField( const QSqlField& other );
    QSqlField& operator=( const QSqlField& other );
    bool operator==(const QSqlField& other) const;
    virtual ~QSqlField();

    virtual void setValue( const QVariant& value );
    virtual QVariant value() const;
    virtual void setName( const QString& name );
    QString name() const;
#ifndef QT_NO_COMPAT
    void setNull();
#endif
    bool isNull() const;
    virtual void setReadOnly( bool readOnly );
    bool isReadOnly() const;
    virtual void clear();
    QVariant::Type type() const;

private:
    QSqlFieldPrivate* d;
};

/******************************************/
/*******     QSqlFieldInfo Class     ******/
/******************************************/

struct QSqlFieldInfoPrivate;

class QM_EXPORT_SQL QSqlFieldInfo
{
public:
    QSqlFieldInfo( const QString& name = QString(),
		   QVariant::Type typ = QVariant::Invalid,
		   int required = -1,
		   int len = -1,
		   int prec = -1,
		   const QVariant& defValue = QVariant(),
		   int sqlType = 0,
		   bool generated = TRUE,
		   bool trim = FALSE,
		   bool calculated = FALSE );
    QSqlFieldInfo( const QSqlFieldInfo & other );
    QSqlFieldInfo( const QSqlField & other, bool generated = TRUE );
    virtual ~QSqlFieldInfo();
    QSqlFieldInfo& operator=( const QSqlFieldInfo& other );
    bool operator==( const QSqlFieldInfo& f ) const;

    QSqlField		toField() const;
    int			isRequired() const;
    QVariant::Type	type() const;
    int			length() const;
    int			precision() const;
    QVariant		defaultValue() const;
    QString		name() const;
    int			typeID() const;
    bool		isGenerated() const;
    bool		isTrim() const;
    bool		isCalculated() const;

    virtual void	setTrim( bool trim );
    virtual void	setGenerated( bool gen );
    virtual void	setCalculated( bool calc );

private:
    QSqlFieldInfoPrivate* d;
};


#endif	// QT_NO_SQL
#endif
