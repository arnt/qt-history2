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
    QSqlDriverInterface( QUnknownInterface *parent = 0 ) : QUnknownInterface( parent ) {}
    QString interfaceID() const { return createID( QUnknownInterface::interfaceID(), "QSqlDriverInterface" ); }

    virtual QSqlDriver* create( const QString& name ) = 0;
    virtual QStringList featureList() const { return QStringList(); }
};

#endif // QT_NO_SQL

#endif // QSQLDRIVERINTERFACE_H
