#include <QObject>

template <typename T>
struct TestTemplate
{
    T *blah;
};

class SlotsWithVoidTemplateTest : public QObject
{
    Q_OBJECT
public slots:
    inline void dummySlot() {}
    inline void anotherSlot(const TestTemplate<void> &) {}
    inline TestTemplate<void> mySlot() { return TestTemplate<void>(); }
signals:
    void mySignal(const TestTemplate<void> &);
    void myVoidSignal();
};
