#ifndef QREMOTEPLUGIN_H
#define QREMOTEPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_REMOTE
class QRemoteInterface;
class QRemotePluginPrivate;

class Q_EXPORT QRemotePlugin : public QGPlugin
{
    Q_OBJECT
public:
    QRemotePlugin();
    ~QRemotePlugin();

    virtual QStringList keys() const = 0;
    virtual QRemoteInterface *create( const QString &key ) = 0;

private:
    QRemotePluginPrivate *d;
};
#endif // QT_NO_REMOTE
#endif // QREMOTEPLUGIN_H
