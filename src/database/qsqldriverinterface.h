#ifndef QSQLDRIVERINTERFACE_H
#define QSQLDRIVERINTERFACE_H

#ifndef QT_H
#include "qcomponentinterface.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDriver;
class Q_EXPORT QSqlDriverInterface : public QUnknownInterface
{
public:
    virtual QSqlDriver* create( const QString& name ) = 0;

    virtual QStringList featureList() = 0;

    QString interfaceID() { return "QSqlDriverInterface"; }
};

#endif // QT_NO_SQL

#endif // QSQLDRIVERINTERFACE_H
