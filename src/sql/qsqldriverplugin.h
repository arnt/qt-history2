#ifndef QSQLDRIVERPLUGIN_H
#define QSQLDRIVERPLUGIN_H

#ifndef QT_H
#include "qobject.h"
#include "qstringlist.h"
#include "qplugin.h"
#endif // QT_H

class QSqlDriver;
class QSqlDriverPluginPrivate;
struct QUnknownInterface;

class Q_EXPORT QSqlDriverPlugin : public QObject
{
public:
    QSqlDriverPlugin();
    virtual ~QSqlDriverPlugin();

    virtual QStringList featureList() const;
    virtual QSqlDriver *create( const QString &key );

    QUnknownInterface *iface();

private:
    QSqlDriverPluginPrivate *d;
};

#endif // QSQLDRIVERPLUGIN_H
