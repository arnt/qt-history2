#ifndef QSQLDRIVERPLUGIN_H
#define QSQLDRIVERPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

class QSqlDriver;
class QSqlDriverPluginPrivate;

class Q_EXPORT QSqlDriverPlugin : public QGPlugin
{
public:
    QSqlDriverPlugin();
    ~QSqlDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QSqlDriver *create( const QString &key ) = 0;

private:
    QSqlDriverPluginPrivate *d;
};

#endif // QSQLDRIVERPLUGIN_H
