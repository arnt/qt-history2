
#include <QObject>

namespace AAA {
    struct BaseA {};
}

namespace BBB {
    class Foo : public QObject, public ::AAA::BaseA
    {
        Q_OBJECT
    };
}

