#ifndef QMSNETLIBHEAD_H
#define QMSNETLIBHEAD_H

#include <qobject.h>

class QMSNETLIBCLASS : public QObject
{
    Q_OBJECT

public:
    QMSNETLIBCLASS( QObject *parent = 0, const char *name = 0 );
    ~QMSNETLIBCLASS();

};

#endif // QMSNETLIBHEAD_H
