#ifndef QQUEUE_H
#define QQUEUE_H

#ifndef QT_H
#include "qlist.h"
#endif // QT_H

template <class T>
class QQueue : public QList<T>
{
public:
    inline QQueue() {}
    inline ~QQueue() {}
    inline void enqueue(const T &t) { append(t); }
    inline T dequeue() { return takeFirst(); }
    inline T &head() { return first(); }
    inline const T &head() const { return first(); }
};

#endif
