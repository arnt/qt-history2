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

class QSqlExtension {
public:
    virtual ~QSqlExtension();
    virtual bool prepare( const QString& query );
    virtual bool exec();
    virtual void setValue( const QString& holder, const QVariant& value );
    
    QMap<QString, QVariant> values;
};

#endif
#endif
