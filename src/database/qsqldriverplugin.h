#ifndef QSQLDRIVERPLUGIN_H
#define QSQLDRIVERPLUGIN_H

#ifndef QT_H
#include "qplugin.h"
#include "qpluginmanager.h"
#include "qsqldriverinterface.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlDriverPlugIn : public QSqlDriverInterface, public QPlugIn
{
public:
    QSqlDriverPlugIn( const QString& filename, LibraryPolicy = Default, const char* fn=0 );
    QString queryInterface() const { return "QSqlDriverInterface"; }
    QStringList featureList();

    QSqlDriver* create( const QString& name );
};

class Q_EXPORT QSqlDriverPlugInManager : public QPlugInManager< QSqlDriverPlugIn >
{
public:
    QSqlDriverPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
			     QPlugIn::LibraryPolicy pol = QPlugIn::Default, const char* fn = 0 );
    ~QSqlDriverPlugInManager()
    {
    }
};

#endif // QT_NO_SQL

#endif

