#ifndef QSTYLEFACTORY_H
#define QSTYLEFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

class QString;
class QStyle;

class Q_EXPORT QStyleFactory
{
public:
    static QStyle *create( const QString& );
    static QStringList styles();
};

#endif //QSTYLEFACTORY_H
