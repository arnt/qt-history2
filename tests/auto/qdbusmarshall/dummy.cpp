#include <QtTest>

class Dummy: public QObject
{
    Q_OBJECT
private slots:
    void QtDBusIsNotEnabled()
    {
        QSKIP("QtDBus is not enabled in this platform", SkipAll);
    }
};

QTEST_MAIN(Dummy)
#include "dummy.moc"
