#include <QTimer>

class Foo : public QObject
{
public:
    Foo();
};

Foo::Foo()
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateCaption()));
    timer->start(1000);

    QTimer::singleShot(200, this, SLOT(updateCaption()));

    {
    // ZERO-CASE
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(processOneThing()));
    timer->start();
    }
}

int main()
{
    
}
