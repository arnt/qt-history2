#ifndef QCOMPONENTFACTORY_H
#define QCOMPONENTFACTORY_H

#ifndef QT_H
#include <qcom.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

class Q_EXPORT QComponentFactory
{
public:
    static QRESULT createInstance( const QUuid &cid, const QUuid &iid, QUnknownInterface** instance, QUnknownInterface *outer = 0 );
    static bool registerServer( const QString &filename );
    static bool unregisterServer( const QString &filename );

    static bool registerComponent( const QUuid &cid, const QString &filename, const QString &description = QString::null );
    static bool unregisterComponent( const QUuid &cid );
};

#endif // QT_NO_COMPONENT

#endif // QCOMPONENTFACTORY_H
