#ifndef QSTYLEFACTORY_H
#define QSTYLEFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_STYLE

class QString;
class QStyle;

class Q_EXPORT QStyleFactory
{
public:
#ifndef QT_NO_STRINGLIST
    static QStringList keys();
#endif
    static QStyle *create( const QString& );
};

#endif //QT_NO_STYLE

#endif //QSTYLEFACTORY_H
