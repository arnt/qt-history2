#ifndef LIBMAIN_H
#define LIBMAIN_H

#include <qobject.h>

class LibMain : public QObject
{
    Q_OBJECT
public:
    LibMain( QObject *parent = 0, const char *name = 0 );
    ~LibMain();
};

#endif // LIBMAIN_H
