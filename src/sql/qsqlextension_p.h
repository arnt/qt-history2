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
#include "qstring.h"
#include "qvariant.h"
#endif // QT_H

#ifndef QT_NO_SQL

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#endif

#if defined(Q_TEMPLATEDLL)
Q_TEMPLATE_EXTERN template class QM_EXPORT_SQL QMap<QString,QVariant>;
#endif

class QM_EXPORT_SQL QSqlExtension {
public:
    virtual ~QSqlExtension();
    virtual bool prepare( const QString& query );
    virtual bool exec();
    virtual void bindValue( const QString& holder, const QVariant& value );
    virtual void bindValue( int pos, const QVariant& value );
    virtual void addBindValue( const QVariant& value );
    virtual void clearValues();
    
    QVariant value( const QString& holder );
    QVariant value( int i );
    
    QMap<int, QString> index;
    QMap<QString, QVariant> values;
};

#endif
#endif
