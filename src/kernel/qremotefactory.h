#ifndef QREMOTEFACTORY_H
#define QREMOTEFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

#ifdef QT_REMOTE_CONTROL

class QString;
class QRemoteInterface;

class Q_EXPORT QRemoteFactory
{
public:
#ifndef QT_NO_STRINGLIST
    static QStringList keys();
#endif
    static QRemoteInterface *create( const QString& );
};

#endif //QT_REMOTE_CONTROL

#endif //QREMOTEFACTORY_H
