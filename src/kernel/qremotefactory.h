#ifndef QREMOTEFACTORY_H
#define QREMOTEFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_REMOTE

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

#endif //QT_NO_REMOTE

#endif //QREMOTEFACTORY_H
