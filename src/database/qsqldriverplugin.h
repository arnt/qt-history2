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
    QSqlDriverPlugIn( const QString& filename, QApplicationInterface* = 0, LibraryPolicy = Default );
    QString queryInterface() const { return "QSqlDriverInterface"; }

    QSqlDriver* create( const QString& name );
};

class Q_EXPORT QSqlDriverPlugInManager : public QPlugInManager< QSqlDriverPlugIn >
{
public:
    QSqlDriverPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
			     QApplicationInterface* = 0, QPlugIn::LibraryPolicy pol = QPlugIn::Default );
    ~QSqlDriverPlugInManager()
    {
    }
};

#endif // QT_NO_SQL

#endif

