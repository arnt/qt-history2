#ifndef QSQLDRIVERPLUGIN_H
#define QSQLDRIVERPLUGIN_H

#ifndef QT_H
#include "qplugin.h"
#include "qpluginmanager.h"
#include "qsqldriverinterface.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlDriverPlugInManager : public QInterfaceManager< QSqlDriverInterface >
{
public:
    QSqlDriverPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
			     QApplicationInterface* = 0, QPlugIn::LibraryPolicy pol = QPlugIn::Default );

    QSqlDriver* create( const QString& name );
};

#endif // QT_NO_SQL

#endif
