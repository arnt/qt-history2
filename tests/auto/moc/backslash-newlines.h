
#include <QObject>

const int blackslashNewlinesDummy = 0

#define value 0\
1

;

class BackslashNewlines : public QObject
{
    Q_OBJECT
public slots:
#if value
    void works() {}
#else
    void buggy() {}
#endif
};

#undef value

