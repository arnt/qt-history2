#ifndef QGFXDRIVERFACTORY_H
#define QGFXDRIVERFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

class QString;
class QScreen;

class Q_EXPORT QGfxDriverFactory
{
public:
#ifndef QT_NO_STRINGLIST
    static QStringList keys();
#endif
    static QScreen *create( const QString&, int );
};

#endif //QGFXDRIVERFACTORY_H
