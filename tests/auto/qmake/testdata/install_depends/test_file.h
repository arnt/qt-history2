#include <qobject.h>

class SomeObject : public QObject
{
    Q_OBJECT
public:
    SomeObject();
signals:
    void someSignal();
};
