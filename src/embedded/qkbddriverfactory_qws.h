#ifndef QKBDDRIVERFACTORY_H
#define QKBDDRIVERFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

class QString;
class QWSKeyboardHandler;

class Q_EXPORT QKbdDriverFactory
{
public:
#ifndef QT_NO_STRINGLIST
    static QStringList keys();
#endif
    static QWSKeyboardHandler *create( const QString& );
};

#endif //QKBDDRIVERFACTORY_H
