#include "qsqlextension_p.h"

#ifndef QT_NO_SQL

QSqlExtension::~QSqlExtension()
{
}

bool QSqlExtension::prepare( const QString& /*query*/ )
{
    values.clear();
    return FALSE;
}

bool QSqlExtension::exec()
{
    return FALSE;
}

void QSqlExtension::setValue( const QString& placeholder, const QVariant& val )
{
    values[ placeholder ] = val;
}

#endif
