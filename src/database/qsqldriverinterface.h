#ifndef QSQLDRIVERINTERFACE_H
#define QSQLDRIVERINTERFACE_H

#ifndef QT_H
#include <qplugininterface.h>
#include <qstringlist.h>
#endif // QT_H

class QSqlDriver;
class QSqlDriverInterface : public QPlugInInterface
{
public:
    virtual QSqlDriver* create( const QString& name ) = 0;
};

#endif // QSQLDRIVERINTERFACE_H
