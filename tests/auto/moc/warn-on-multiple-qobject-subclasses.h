
#include <QtGui>

class Foo : public QObject
{
    Q_OBJECT
public:
};

class Bar : public QWidget, public Foo
{
    Q_OBJECT
};


