#ifndef QMOUSEDRIVERFACTORY_H
#define QMOUSEDRIVERFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

class QString;
class QWSMouseHandler;

class Q_EXPORT QMouseDriverFactory
{
public:
#ifndef QT_NO_STRINGLIST
    static QStringList keys();
#endif
    static QWSMouseHandler *create( const QString&, const QString & );
};

#endif //QMOUSEDRIVERFACTORY_H
