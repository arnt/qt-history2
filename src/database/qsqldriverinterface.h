#ifndef QSQLDRIVERINTERFACE_H
#define QSQLDRIVERINTERFACE_H

#include <qplugininterface.h>
#include <qstringlist.h>

class QSqlDriver;
class QSqlDriverInterface : public QPlugInInterface
{
public:
    virtual QSqlDriver* create( const QString& name ) = 0;
};

#endif