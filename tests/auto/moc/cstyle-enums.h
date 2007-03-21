#include <QObject>

class CStyleEnums
{
    Q_GADGET
public:
    Q_ENUMS(Baz)
    typedef enum { Foo, Bar } Baz;
};

