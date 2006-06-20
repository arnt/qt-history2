#include <QtCore/qobject.h>

class Foo: public QObject
{
    Q_OBJECT
public:


signals:

public slots:
    inline void foo() {}
    inline int bar(int, int*, const int *, volatile int *, const int * volatile *) { return 0; }
    inline void slot(int, QObject * const) {}
};

